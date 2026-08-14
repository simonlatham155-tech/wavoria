# Latham Audio Wavoria

**Fifty years of synthesis that never happened.**

Wavoria imagines what wave-terrain synthesis might have become if the idea had received five decades of continuous instrument design, musical refinement, and engineering—not merely survived as an oscillator mode inside a conventional subtractive synth.

The terrain is the instrument. Every voice travels through one shared mathematical world. Notes deform it, chords create interacting paths, gravity bends later trajectories toward earlier gestures, and the field remembers before slowly returning to rest.

## What makes it Wavoria

- **One shared terrain** — all 12 voices read and write the same dynamic field.
- **Dual interacting readers** — each note follows a primary orbit and a coupled satellite path.
- **Note-driven deformation** — velocity, pressure, mod wheel, and envelope energy leave physical imprints.
- **Topographic memory** — the surface can forget immediately or retain a performance for almost a minute.
- **Field gravity** — readers respond to the gradient created by earlier notes and other voices.
- **Recallable evolution** — a field seed makes the same performance history repeatable.
- **The globe is the engine view** — live reader trails and field energy are drawn from the synthesis state.

This is intentionally different from adding “wave terrain” to an oscillator selector and then routing it through a familiar synth. Terrain, trajectory, deformation, interaction, memory, and polyphony are the core signal path.

## Current instrument

The first playable architecture includes:

- 12-voice sample-accurate MIDI polyphony with voice stealing
- sustain pedal, pitch bend, poly aftertouch, channel pressure, and mod-wheel expression
- four continuously morphing terrain families: dunes, basins, lattice, and crystal
- circular-to-Lissajous trajectory morphing, radius, rotation, drift, and reader interaction
- shared note imprints, memory decay, and gradient gravity
- ADSR, tone control, nonlinear drive, stereo voice field, and output level
- eight factory sounds plus editable user presets stored in Wavoria's own preset format
- Veloria-family Discover, New Field, Save, Save As, Rename, Delete, and previous/next workflow
- VST3, Audio Unit, and standalone targets through JUCE
- dependency-free DSP tests for bounds, determinism, field memory, chord deformation, and release behavior

## Visual identity

Wavoria keeps the dark glass/metal construction and information hierarchy of the Latham Audio instrument family, while using its own **electric-jade / deep-lagoon** signature with restrained molten-coral energy accents. The central mineral globe shows the shared terrain, active readers, fading paths, and accumulated field state; it is not a decorative oscilloscope.

## Build

Wavoria uses C++20, JUCE 8.0.4, and CMake 3.22 or newer.

```bash
git clone https://github.com/simonlatham155-tech/wavoria.git
cd wavoria
cmake -S . -B build
cmake --build build --config Release
```

JUCE is fetched automatically during configuration. To build only the dependency-free DSP tests:

```bash
cmake -S . -B build-tests -DWAVORIA_BUILD_PLUGIN=OFF -DWAVORIA_BUILD_TESTS=ON
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

## Documentation

- [Engine architecture](docs/engine.md)
- [Product roadmap](docs/roadmap.md)
- [Instrument design language](docs/design.md)

## Status

The shared-field engine, polyphonic MIDI layer, factory/user preset workflow, state handling, and first complete instrument interface are implemented. The next milestone is deeper sound design, alias-control profiling, preset search/favourites, and host validation across the three target formats.
