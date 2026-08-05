# USAM — Universal Sound Asset Manager

USAM is an all-in-one music creation platform built around four pillars:

| Pillar | Product class | Status |
|---|---|---|
| **Synthesizer** | Professional polyphonic hybrid synth (Serum 2 / Omnisphere 2 / Vital / Pigments class) | 🚧 In development (M0 → M1) |
| **Sample Player** | Multi-sampled instrument player (Kontakt 8 class) | Planned (M2) |
| **Sampler** | Loop chopping / beat-making sampler (Serato Sample class) | Planned (M3) |
| **Library Manager** | Sound-asset database & browser — samples, presets, instruments, plugins, projects (Sononym / Native Access class) | Planned (M4) |

The core insight: **one database, one browser, one patch format, one engine.** Any sound
asset — a wavetable, a multi-sampled piano, a sliced break — is content that flows through the
same library, the same engine, and the same format.

See [ARCHITECTURE.md](ARCHITECTURE.md) for the full system design and
[ROADMAP.md](ROADMAP.md) for the milestone plan.

## Repository layout

```
usam/
├── audio/            # JUCE C++ audio engine (current focus)
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── core/     # plugin processor, parameters, voices, voice manager
│   │   ├── dsp/      # oscillators, filters, envelopes
│   │   └── gui/      # plugin editor
│   └── tests/        # JUCE UnitTest-based DSP & engine tests
├── ARCHITECTURE.md   # full platform architecture
└── ROADMAP.md        # milestones M0–M5
```

The Electron/React library manager described in ARCHITECTURE.md §14 will be built in a later
milestone (M4) under `manager/`; an earlier scaffold was removed to keep the repo focused on
the engine.

## Building the engine

Requirements: CMake ≥ 3.22 and a C++20 toolchain (Visual Studio 2022 on Windows, Xcode on
macOS, GCC/Clang on Linux). JUCE 9 is fetched automatically via CMake `FetchContent`.

```sh
cmake -S audio -B audio/build
cmake --build audio/build --config Release
```

This produces:

- **USAM Engine** — VST3 plugin + Standalone app
- **USAMUnitTests** — console unit-test runner

## Running the tests

```sh
cmake --build audio/build --config Release --target USAMUnitTests
./audio/build/USAMUnitTests_artefacts/Release/"USAM Unit Tests"
```

The runner exits non-zero on failure, so it can gate CI.
