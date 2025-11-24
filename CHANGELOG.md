# Changelog

All notable changes to the Precision Guitar Tuner project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- **Pitch Stabilization**: Three stabilization algorithms (EMA, Median, Hybrid) to reduce jittery pitch readings
  - Exponential Moving Average for general smoothing
  - Median Filter for spike rejection
  - Hybrid (recommended) combines both with confidence weighting
  - Configurable via AudioProcessingLayer::Config
  - All implementations are real-time safe (no allocations)

### Added

- **Audio Feedback Features** (2025-11-24)
  - **Separate Input/Output Devices**: Independent audio device selection for input (guitar) and output (monitoring/feedback)
  - **Reference Tone Generator**: Adjustable sine wave reference tone (100-1000 Hz) with volume control
  - **Input Monitoring (Digital Amp)**: Real-time audio monitoring with adjustable volume (0-500%) and input gain (0-500%)
  - **Stereo Output Support**: Automatic detection and configuration of stereo output devices
  - **Thread-Safe Audio Processing**: All audio feedback settings use atomic operations for safe real-time updates
  - **UI Controls**: Integrated settings panel with device selectors and audio feedback controls
  
- **Configuration** (2025-11-24)
  - Added `AudioConfig` fields: `outputDeviceId`, `outputDeviceName`, `enableReference`, `referenceVolume`, `referenceFrequency`, `enableInputMonitoring`, `monitoringVolume`, `inputGain`
  - Configuration persistence for all audio feedback settings

- **Features** (2025-11-24)
  - **Multiple Tuning Modes**: Added support for 7 tuning presets (Standard, Drop D, Drop C, DADGAD, Open G, Open D, Chromatic)
  - **TuningPresets Class**: Implemented static class for managing tuning definitions and frequency calculations
  - **Unit Tests**: Added comprehensive test suite for tuning presets covering all modes and edge cases

- **Maintainability** (2025-11-23)
  - Centralized window size constants in `Config.h` as `static constexpr` members of `WindowConfig`
  - Added pre-allocation of `YinPitchDetector` buffers during initialization to ensure real-time safety
  - Created basic test infrastructure with `tests/CMakeLists.txt` and `tests/ConfigTest.cpp`

### Changed

- **Testing Infrastructure** (2025-11-24)
  - **Google Test Integration**: Refactored all unit tests to use Google Test framework
  - **Test Refactoring**: Converted `ConfigTest` and `TuningPresetsTest` from manual assertions to GTest macros
  - **Build System**: Updated `tests/CMakeLists.txt` to link against `GTest::gtest` and use `gtest_discover_tests`

- **C++20 Modernization** (2025-11-23)
  - Updated `lib-guitar-io` to use `std::span<const float>` and `std::span<float>` for audio buffers instead of raw pointers
  - Updated `lib-guitar-dsp` `PitchDetector::Detect` to accept `std::span<const float>` instead of pointer + size
  - Updated `AudioProcessingLayer` to match new library interfaces
  - Entire audio pipeline now uses `std::span` for type safety and bounds checking
  - Replaced manual loop in `SettingsLayer` with `std::ranges::find_if` for device selection

### Fixed

- **Bug Fixes** (2025-11-24)
  - Fixed crash in `TuningPresets::CalculatePreset` when accessing empty note names in Chromatic mode

- **Security** (2025-11-23)
  - Added null check for `getpwuid` result in `Config.cpp` to prevent potential crash on Linux

### Removed

- **Dead Code** (2025-11-23)
  - Removed unused `FontRenderer.cpp` and `FontRenderer.h` (functionality replaced by ImGui)

## [0.0.1] - 2025-11-23 (Unreleased)

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
  - Repository: <https://github.com/Konstantysz/lib-guitar-io>

- **lib-guitar-dsp (v0.0.1)** - Digital signal processing library
  - YIN pitch detection algorithm implementation
  - NoteConverter utilities (frequency ↔ note conversion)
  - PFFFT integration (BSD-licensed FFT)
  - Real-time optimized algorithms
  - BSD License (GPL-free)
  - Repository: <https://github.com/Konstantysz/lib-guitar-dsp>

- **AudioProcessingLayer**
  - Real-time audio capture at 48kHz
  - YIN pitch detection with ±0.1 cent target accuracy
  - Lock-free atomic communication with UI thread
  - Zero-allocation audio callback
  - Configurable frequency range (80-1200 Hz)
  - 2048-frame buffer size for accuracy

- **TunerVisualizationLayer** - Modern OpenGL visual tuner
  - GLSL 330 vertex/fragment shaders (Core Profile)
  - GPU-accelerated rendering with VBOs and VAOs
  - Real-time 60 FPS visualization
  - Horizontal cent deviation meter (±50 cent range with tick marks)
  - Large circular tuning indicator with color-coded feedback:
    - Green (±0-3 cents): In tune
    - Yellow-Green (±3-10 cents): Close
    - Orange (±10-25 cents): Adjust more
    - Red (>±25 cents): Out of tune
  - "IN TUNE" indicator with pulsing green border (within ±3 cents)
  - Smooth color transitions using GLM color mixing

- **TrueType Font Rendering with stb_truetype**
  - Professional text rendering using stb_truetype single-header library
  - Font texture atlas generation (512x512) for ASCII characters
  - Antialiased text with proper kerning
  - Automatic system font loading (Arial, Consola, Helvetica, DejaVu Sans)
  - FontRenderer class with OpenGL texture-based rendering
  - Note name display with octave (e.g., "E4", "A2")
  - Frequency display in Hz (e.g., "440.0 Hz")
  - Precise cent deviation display (e.g., "+2.3", "-1.5")
  - Text color-coded to match tuning indicator

- **Dual Shader System**
  - Geometry shader program for shapes (circles, rectangles)
  - Text shader program with texture sampling for fonts
  - Separate VAO/VBO for geometry and text rendering
  - Alpha blending for smooth text antialiasing
  - External GLSL shader files (assets/shaders/)

- **Code Organization**
  - Namespace structure: `PrecisionTuner::Layers` for layer classes
  - Organized src structure: `src/Layers/` for layer implementations
  - Main entry point renamed to `PrecisionGuitarTuner.cpp`
  - CMake automatic asset copying to build directory

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
