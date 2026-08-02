# USAM Engine (audio/)

The native audio plugin for USAM — a JUCE 9 (C++20) synthesizer/sampler engine.
This is the M0 scaffold: a working polyphonic wavetable synth that loads in a DAW
as **VST3** and runs standalone.

## Status (M0)

- ✅ CMake + JUCE 9.0.0 scaffold (VST3 + Standalone targets)
- ✅ APVTS parameter tree (oscillator, filter, amp envelope, master gain)
- ✅ Polyphonic voice manager (round-robin allocation, simple stealing)
- ✅ Wavetable oscillator (sine / saw / square / triangle)
- ✅ State-variable filter (LP / HP / BP) with parameter smoothing
- ✅ ADSR envelope with velocity scaling
- ✅ Patch save/load via host state (XML)
- ✅ Unit tests (console runner, CI-friendly exit codes)

## Prerequisites

- CMake ≥ 3.22 (VS 18 / VS 2022 include one)
- A C++20 compiler (MSVC 19.5+, Clang, or GCC)
- Internet access on first configure (FetchContent downloads JUCE 9.0.0)

## Build

```sh
cmake -S . -B build -G "Visual Studio 18 2026" -A x64   # or your generator
cmake --build build --config Release --target USAMEngine_VST3 USAMEngine_Standalone
```

Artefacts land in `build/USAMEngine_artefacts/Release/`:

- `VST3/USAM Engine.vst3` — load in any VST3-capable DAW
- `Standalone/USAM Engine.exe` — runs standalone (MIDI/audio via JUCE)

## Test

```sh
cmake --build build --config Release --target USAMUnitTests
./build/USAMUnitTests_artefacts/Release/USAMUnitTests.exe
```

Exits `0` when all tests pass, `1` on any failure.

## Layout

```
src/
├── core/       PluginProcessor, VoiceManager, SynthVoice, SynthParameters
├── dsp/        oscillators, filters, envelopes
├── gui/        plugin editor
└── presets/    (next milestone: file-based patch save/load)
tests/          JUCE unit tests
```

## Next (M1)

Unison, full filter bank (7 categories), 12 envelopes, 8 LFOs, 16×16 modulation
matrix, arpeggiator, and the effect rack. See `../ARCHITECTURE.md` §8 for the
full spec.
