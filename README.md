# Serialist

Serialist is a C++17 header-only library for building modular real-time generative MIDI systems. 

### Key Concepts
- Step Sequencing & pattern-based generation 
- Multichannel generation with implicit broadcasting
- Phase generators, Waveforms and traditional DSP approaches
- Rule-based and stochastic approaches
- Transport & scheduling in mixed time formats


## Project Status

The project is currently in development and does not have a stable API. Expect breaking changes.

The GUI submodule (located in `src/serialist/gui/` is not maintained and will be moved to a separate repository before the 1.0 release.


## Building
Serialist is a header-only library. To use it in your project:

```cmake
add_subdirectory(path/to/serialist)
target_link_libraries(my_other_project PUBLIC serialist::core)
```

To run the library's test suite:

```bash
git clone --recursive git@github.com:jobor019/serialist.git
cd serialist
cmake -S . -B build
cmake --build build --target core_tests testutils_tests
ctest --test-dir build --output-on-failure
```

Examples will be available soon. See the [max-serialist](https://github.com/jobor019/serialist-max) repo for concrete examples.

## Architecture

### Project Structure

```
src/serialist/core/
├── algo/          # Misc. statistical and stochastic algorithms
├── collections/   # Specialized containers and data structures
├── generatives/   # Core `Generative` modules
├── param/         # Parameter handling, see `policies/` below
├── policies/      # Overridable behaviour policies for GUI integration
├── temporal/      # Transport, scheduling, and time-dependent algorithms
├── types/         # Fundamental data types and concepts
└── utility/       # Common utilities and type traits
```

### Generatives

The project is structured around the modules in the `generatives/` folder, which are interconnectable modular blocks inheriting from the `Generative` base class. 

Each module has a fixed set of multi-channelinputs and (typically) a single multi-channel output, where the output will be broadcast to match the number of voices in each input. The modulescan be directly connected to each other to form a processing graph which should be polled by a `Transport` object at regular intervals (typically 1-10ms).  

The following modules are currently available:

#### Signal Generation

- PhaseNode: Multi-period phase generation
- RandomNode: Various stochastic signal sources

### Pulse Generation & Processing
- PhasePulsatorNode: Phase-synchronized pulse generation
- PulseFilterNode: Pulse sustaining and filtering 

### Signal Processing
- LowPassNode: Basic filtering for signal smoothing
- OperatorNode: Basic mathematical operations between signals
- PhaseMapNode: Mapping a single phase to several shorter subphases
- SampleAndHoldNode: Gated sampling-and-hold
- WaveformNode: Traditional waveform mappings (sine, saw, square, etc.)

### Sequencing & Patterns
- IndexNode: Stepped index generation for sequencing
- InterpolatorNode: Corpus-based interpolation
- PatternizerNode: Pattern transformation

### State and Parameters
- Sequence: Multi-value parameter storage
- Variable: Single-value parameter storage

### MIDI Generation
- MakeNoteNode: Convert signals to MIDI note events

Note that the code in these classes is not designed to be lock-free, and should never be called directly from a real-time audio thread.

## License

Once the first official release (1.0.0) is released, the library will be distributed under the MIT license.