# USAM

Universal Sound Asset Manager — an all-in-one music creation platform
(synthesizer + sample player + loop sampler + library manager).

## Current focus

The native audio engine in `audio/` (JUCE 9, C++20, CMake). The Electron/React
manager described in ARCHITECTURE.md §14 comes later (milestone M4); do not
reintroduce Node/TypeScript scaffolding before then.

## Build & test

```sh
cmake -S audio -B audio/build
cmake --build audio/build --config Release
./audio/build/USAMUnitTests_artefacts/Release/"USAM Unit Tests"
```

## Coding rules

- C++20, JUCE style (4-space indent, space before `(`, `juce::` types).
- The audio thread is sacred: no allocation, no locks, no logging, no file I/O
  in anything reachable from `processBlock`. Preallocate in `prepare*`.
- DSP primitives live in `audio/src/dsp` and must be testable without a host;
  voice/engine logic in `audio/src/core`.
- Every DSP or engine change ships with a JUCE UnitTest in `audio/tests`.
- Parameters flow one way: APVTS → `SynthParameters` snapshot → voices.
  Never read APVTS below `PluginProcessor`.
- No hardcoded paths.
