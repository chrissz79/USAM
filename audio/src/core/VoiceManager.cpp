#include "VoiceManager.h"

namespace usam
{

void VoiceManager::prepare (double sampleRate, int polyphony, int maxBlockSize)
{
    this->sampleRate = sampleRate;
    const int cappedPolyphony = juce::jlimit (1, 64, polyphony);

    voices.clear();
    voices.reserve (static_cast<size_t> (cappedPolyphony));
    for (int i = 0; i < cappedPolyphony; ++i)
    {
        auto voice = std::make_unique<SynthVoice>();
        voice->prepare (sampleRate, maxBlockSize);
        voices.push_back (std::move (voice));
    }

    roundRobinIndex = 0;
}

int VoiceManager::findFreeVoice() noexcept
{
    // Round-robin start so notes spread across the pool instead of always
    // reusing voice 0 first.
    for (size_t i = 0; i < voices.size(); ++i)
    {
        const size_t idx = (static_cast<size_t> (roundRobinIndex) + i) % voices.size();
        if (! voices[idx]->isActive())
        {
            roundRobinIndex = static_cast<int> (idx) + 1;
            return static_cast<int> (idx);
        }
    }
    return -1;
}

int VoiceManager::findVoiceToSteal() noexcept
{
    // Simple oldest-first policy: return the first active voice. A future
    // milestone can prefer voices in the release stage (they die soonest) by
    // exposing release state on SynthVoice.
    for (size_t i = 0; i < voices.size(); ++i)
        if (voices[i]->isActive() && voices[i]->getCurrentNote() >= 0)
            return static_cast<int> (i);

    return -1; // should never happen when all voices are active
}

void VoiceManager::noteOn (int midiNote, float velocity, const SynthParameters& params) noexcept
{
    int index = findFreeVoice();
    if (index < 0)
        index = findVoiceToSteal();
    if (index < 0)
        return; // no voice available (should not happen)

    voices[static_cast<size_t> (index)]->startNote (midiNote, velocity, params);
}

void VoiceManager::noteOff (int midiNote) noexcept
{
    for (auto& voice : voices)
        if (voice->getCurrentNote() == midiNote)
            voice->stopNote();
}

void VoiceManager::allNotesOff() noexcept
{
    for (auto& voice : voices)
        voice->stopNote();
}

void VoiceManager::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample,
                                    int numSamples, const SynthParameters& params) noexcept
{
    for (auto& voice : voices)
        if (voice->isActive())
            voice->renderNextBlock (buffer, startSample, numSamples, params);
}

int VoiceManager::getNumActiveVoices() const noexcept
{
    int count = 0;
    for (const auto& voice : voices)
        if (voice->isActive())
            ++count;
    return count;
}

} // namespace usam
