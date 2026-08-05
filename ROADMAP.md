# USAM Roadmap

USAM is built engine-first: the native JUCE synthesizer/sampler engine comes before the
library manager, so every later pillar has a solid real-time foundation to plug into.
Full details per milestone live in [ARCHITECTURE.md §24](ARCHITECTURE.md).

| Phase | Scope | Exit criteria |
|---|---|---|
| **M0 — Foundation** ✅ | CMake/JUCE scaffold, plugin loads in a DAW, APVTS, voice manager, 1 wavetable osc + SVF + ADSR, basic patch save/load | Patch round-trips; 1 osc plays in a host |
| **M1 — Synth core** 🚧 | 2 osc + sub/noise, unison, filter categories, 12 envelopes, 8 LFOs, mod matrix, arp, FX rack, plugin GUI v1 | Synth spec (§8) largely met; performance budgets (§18) met |
| **M2 — Sampler engine** | Zones/groups/instruments, mapping editor, round-robin, velocity layers, disk streaming, USL runtime, `.usami` format | Kontakt-class instrument loads & plays; streaming budget met |
| **M3 — Slicer** | Beat/transient detection, pads, keyboard mode, pitch/time, beatgrid, local stem separation | Serato-class slicing workflow in host |
| **M4 — Manager & bridge** | Desktop manager app (Electron/React, rebuilt), bridge server, engine client, FTS5 + AI search | Library browse/tag/search works from inside the plugin |
| **M5 — Polish & release** | Multi-part patches, hardware profiles, MPE, packaging, installers | v1.0 ships for VST3/AU/AAX + manager installers |

## Current focus (M0 → M1)

- ✅ M0 hardening: parameter wiring, real-time safety, test coverage.
- ✅ Band-limited oscillators (mipmapped wavetables, 11 mip levels, alias-free).
- ✅ Dual oscillators + sub + noise layers; stereo voice path.
- ✅ Unison: 1–16 voices per oscillator, symmetric detune, equal-power stereo
  spread, phase fan-out at note start.
- Next M1 bricks, in order: filter categories (more responses + drive),
  LFOs, modulation matrix, FX rack, GUI v1.
