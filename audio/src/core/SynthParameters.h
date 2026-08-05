// SynthParameters
// -----------------------------------------------------------------------------
// Single source of truth for parameter IDs, ranges and the plain-data struct
// that is pushed to voices on the audio thread (lock-free: values only).
#pragma once

#include "dsp/osc/WavetableOscillator.h"

namespace usam
{

// -- APVTS parameter IDs (also used by the preset format) -------------------
namespace ParamID
{
    // Oscillator 1
    inline constexpr const char* osc1Waveform = "osc1Waveform";
    inline constexpr const char* osc1Coarse = "osc1Coarse";           // semitones
    inline constexpr const char* osc1Detune = "osc1Detune";           // cents
    inline constexpr const char* osc1Level = "osc1Level";             // 0..1
    inline constexpr const char* osc1Unison = "osc1Unison";           // 1..16 voices
    inline constexpr const char* osc1UnisonDetune = "osc1UnisonDetune"; // cents
    inline constexpr const char* osc1UnisonSpread = "osc1UnisonSpread"; // 0..1 stereo

    // Oscillator 2
    inline constexpr const char* osc2Waveform = "osc2Waveform";
    inline constexpr const char* osc2Coarse = "osc2Coarse";
    inline constexpr const char* osc2Detune = "osc2Detune";
    inline constexpr const char* osc2Level = "osc2Level";
    inline constexpr const char* osc2Unison = "osc2Unison";
    inline constexpr const char* osc2UnisonDetune = "osc2UnisonDetune";
    inline constexpr const char* osc2UnisonSpread = "osc2UnisonSpread";

    // Sub & noise layers
    inline constexpr const char* subLevel = "subLevel";               // 0..1
    inline constexpr const char* noiseLevel = "noiseLevel";           // 0..1

    // Filter
    inline constexpr const char* filterCutoff = "filterCutoff";       // 20..20000 Hz
    inline constexpr const char* filterResonance = "filterResonance"; // Q 0.1..20
    inline constexpr const char* filterType = "filterType";           // 0=LP 1=HP 2=BP

    // Amp envelope
    inline constexpr const char* ampAttack = "ampAttack";
    inline constexpr const char* ampDecay = "ampDecay";
    inline constexpr const char* ampSustain = "ampSustain";
    inline constexpr const char* ampRelease = "ampRelease";

    // Master
    inline constexpr const char* masterGain = "masterGain";           // dB
} // namespace ParamID

/** Per-oscillator slice of the parameter snapshot. */
struct OscillatorParameters
{
    dsp::WavetableOscillator::Waveform waveform = dsp::WavetableOscillator::Waveform::sine;
    float coarseSemitones = 0.0f;   // -24..24
    float detuneCents = 0.0f;       // -100..100 fine detune
    float level = 0.0f;             // 0..1 (osc1 defaults to 0.8 in the layout)
    int unisonVoices = 1;           // 1..16
    float unisonDetuneCents = 20.0f; // max detune of the outermost unison voices
    float unisonSpread = 0.5f;      // 0 = mono, 1 = full stereo spread
};

/** Plain-data snapshot of the parameters relevant to voice rendering.
    Copied by value onto the audio thread each block; no heap, no locks. */
struct SynthParameters
{
    OscillatorParameters osc1;
    OscillatorParameters osc2;
    float subLevel = 0.0f;          // sine, one octave below osc1 pitch
    float noiseLevel = 0.0f;        // white noise
    float filterCutoff = 12000.0f;  // Hz
    float filterResonance = 0.7f;   // Q
    int filterType = 0;             // 0=LP 1=HP 2=BP
    float ampAttack = 0.01f;        // seconds
    float ampDecay = 0.2f;
    float ampSustain = 0.7f;
    float ampRelease = 0.3f;
    float masterGain = -6.0f;       // dB

    SynthParameters()
    {
        osc1.level = 0.8f; // osc1 audible by default; osc2/sub/noise opt-in
    }
};

} // namespace usam
