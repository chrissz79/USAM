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
    inline constexpr const char* oscWaveform = "oscWaveform";
    inline constexpr const char* oscDetune = "oscDetune";       // cents
    inline constexpr const char* oscLevel = "oscLevel";         // 0..1
    inline constexpr const char* filterCutoff = "filterCutoff"; // 20..20000 Hz
    inline constexpr const char* filterResonance = "filterResonance"; // Q 0.1..20
    inline constexpr const char* filterType = "filterType";     // 0=LP 1=HP 2=BP
    inline constexpr const char* ampAttack = "ampAttack";
    inline constexpr const char* ampDecay = "ampDecay";
    inline constexpr const char* ampSustain = "ampSustain";
    inline constexpr const char* ampRelease = "ampRelease";
    inline constexpr const char* masterGain = "masterGain";     // dB
} // namespace ParamID

/** Plain-data snapshot of the parameters relevant to voice rendering.
    Copied by value onto the audio thread each block; no heap, no locks. */
struct SynthParameters
{
    dsp::WavetableOscillator::Waveform oscWaveform = dsp::WavetableOscillator::Waveform::sine;
    float oscDetune = 0.0f;         // cents
    float oscLevel = 0.8f;
    float filterCutoff = 12000.0f;  // Hz
    float filterResonance = 0.7f;   // Q
    int filterType = 0;             // 0=LP 1=HP 2=BP
    float ampAttack = 0.01f;        // seconds
    float ampDecay = 0.2f;
    float ampSustain = 0.7f;
    float ampRelease = 0.3f;
    float masterGain = -6.0f;       // dB
};

} // namespace usam
