# Wavoria roadmap

## Milestone 1 — A terrain that behaves like an instrument

Implemented:

- analytic multi-family terrain
- dual interacting reader trajectories
- 12-voice MIDI renderer
- shared deformable chord field
- topographic memory and gradient gravity
- expressive MIDI input
- state recall and seeded behavior
- Latham Audio interface with a live terrain globe
- eight factory presets and an editable user-preset library
- Discover and New Field sound-design actions
- dependency-free DSP regression tests

Exit work:

- profile aliasing and CPU at 44.1, 48, 96, and 192 kHz
- validate VST3, AU, and standalone builds on macOS and Windows
- tune parameter tapering with a MIDI controller

## Milestone 2 — Ten undeniable sounds

Create ten patches that prove the engine has breadth without leaning on a conventional subtractive architecture:

- bass
- lead
- pad
- pluck
- bell
- percussion
- drone
- texture
- vocal/formant-like terrain
- evolving cinematic field

Expand the preset library with search, favourites, categories, format versioning, and A/B-safe deterministic recall.

## Milestone 3 — Performance geometry

- per-note MPE pitch, pressure, and timbre
- terrain wells that can be placed and moved from the globe
- trajectory drawing and capture
- chord-shape geometry modes
- reader grouping and orbit relationships
- field freeze, clear, and controlled resynthesis

## Milestone 4 — Spatial terrain

- multi-output reader groups
- terrain-derived stereo and surround motion
- tempo-aware field evolution
- modulation routing that uses geometric relationships instead of a generic matrix alone
- optional oversampling quality modes

## Product test

Every new feature must answer at least one of these questions:

- Does it make the terrain more playable?
- Does it deepen interaction between notes?
- Does it make performance history musically useful?
- Does it reveal a sound that a conventional oscillator/filter/mod-matrix design would not naturally produce?

If not, it does not belong in Wavoria merely because another synthesizer has it.
