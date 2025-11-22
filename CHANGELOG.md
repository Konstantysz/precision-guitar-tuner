# Changelog

All notable changes to the Precision Guitar Tuner project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned

- Text rendering for note names and frequency display
- Strobe tuner visualization mode
- Settings layer for audio device selection
- Configuration persistence
- Alternative tuning support (Drop D, chromatic, etc.)

## [0.0.2] - 2025-11-23

### Added

- **Modern OpenGL Rendering System**
  - GLSL 330 vertex/fragment shaders for 2D geometry
  - Core Profile compatible (no deprecated immediate mode)
  - GPU-accelerated rendering with VBOs and VAOs
  - Real-time 60 FPS visualization

- **Visual Tuner Display**
  - Horizontal cent deviation meter (±50 cent range)
  - Tick marks at ±10, ±20, ±30, ±40, ±50 cents
  - Center line indicator for perfect tuning
  - White outline border on meter
  - Always-visible UI (meter displays even without audio input)

- **Color-Coded Tuning Feedback**
  - Dynamic color indicator bar on deviation meter
  - Large circular tuning indicator above meter
  - Smooth color transitions based on tuning accuracy:
    - Green (±0-3 cents): In tune
    - Yellow-Green (±3-10 cents): Close
    - Orange (±10-25 cents): Adjust more
    - Red (>±25 cents): Out of tune
  - Uses GLM color mixing for smooth gradients

- **Rendering Architecture**
  - `DrawFilledRect()`: Efficient rectangle rendering with triangles
  - `DrawOutlineRect()`: Line-based rectangle borders
  - `DrawCircle()`: Triangle fan/line loop circle rendering
  - Lazy OpenGL initialization on first render
  - Proper resource management with VAO/VBO

### Changed

- **TunerVisualizationLayer** completely rewritten for modern OpenGL
  - Removed console-only output
  - Added full visual rendering pipeline
  - Separated background, meter, and indicator rendering

### Technical Highlights

- Modern OpenGL 3.3+ with shaders (Core Profile)
- Dynamic vertex data uploaded per frame
- Efficient GPU-side geometry processing
- Clean separation of concerns (rendering helpers)
- Follows C++20 best practices
- Code formatted with clang-format (Allman style, 120 char limit)

## [0.0.1] - 2025-01-22

### Added
- **Project Infrastructure**
  - CMake build system with C++20 support
  - vcpkg dependency management
  - Git repository with submodule architecture
  - Platform detection (Windows/macOS/Linux)
  - CLAUDE.md development guidelines
  - DEPENDENCIES.md version tracking
  - Development plan with 11-week roadmap

- **lib-guitar-io (v0.0.1)** - Cross-platform audio I/O library
  - AudioDevice class wrapping RtAudio
  - AudioDeviceManager for device enumeration
  - Platform-specific API support:
    - Windows: ASIO/WASAPI
    - macOS: CoreAudio
    - Linux: ALSA
  - Clean C++20 PIMPL interface
  - MIT License (GPL-free)
  - Repository: https://github.com/Konstantysz/lib-guitar-io

- **lib-guitar-dsp (v0.0.1)** - Digital signal processing library
  - YIN pitch detection algorithm implementation
  - NoteConverter utilities (frequency ↔ note conversion)
  - PFFFT integration (BSD-licensed FFT)
  - Real-time optimized algorithms
  - BSD License (GPL-free)
  - Repository: https://github.com/Konstantysz/lib-guitar-dsp

- **AudioProcessingLayer**
  - Real-time audio capture at 48kHz
  - YIN pitch detection with ±0.1 cent target accuracy
  - Lock-free atomic communication with UI thread
  - Zero-allocation audio callback
  - Configurable frequency range (80-1200 Hz)
  - 2048-frame buffer size for accuracy

- **TunerVisualizationLayer**
  - Pitch data reception from audio layer
  - Frequency to note conversion with cent deviation
  - Console logging of detected notes (temporary UI)
  - 10Hz update rate with 70% confidence threshold

### Technical Highlights
- kappa-core v0.5.1 integration (OpenGL application framework)
- Layer-based architecture (SRP: Separation of audio/DSP/UI)
- Real-time audio constraints followed:
  - No allocations in audio callback
  - No locks in audio callback
  - Pre-allocated buffers
  - Lock-free atomic variables for thread communication
- SOLID principles applied throughout
- GPL-free dependency chain (all MIT/BSD licensed)

### Build
- Executable size: 289KB (Release build)
- Build time: ~18 seconds (CMake configure + build)
- Compiler warnings as errors enabled (/WX on MSVC)

## [0.0.0] - 2025-01-22

### Initial
- Project inception
- Market analysis and strategic foundation
- Technical architecture design
- Repository initialization

---

## Version History

- **0.0.1** (2025-01-22): First working audio engine with pitch detection
- **0.0.0** (2025-01-22): Project initialization

## Migration Notes

### From 0.0.0 to 0.0.1
- Added lib-guitar-io and lib-guitar-dsp as git submodules
- Updated CMakeLists.txt to link new libraries
- Created AudioProcessingLayer and TunerVisualizationLayer
- Application now requires audio input device

## Breaking Changes

None yet (pre-release version).

## Deprecations

None yet (pre-release version).
