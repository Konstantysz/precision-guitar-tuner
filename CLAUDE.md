# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Precision Tuner** is a professional-grade desktop guitar tuner application built with modern C++20. It delivers studio-quality ±0.1 cent accuracy with native audio interface support, targeting professional musicians, home recording enthusiasts, and guitar technicians.

**Technology Stack:**
- **Language:** C++20
- **UI Framework:** kappa-core (OpenGL-based application framework)
- **Audio I/O:** RtAudio (via lib-guitar-io submodule)
- **DSP:** PFFFT, YIN & MPM pitch detection (via lib-guitar-dsp submodule)
- **Build System:** CMake 3.20+ with vcpkg
- **Platforms:** Windows (ASIO/WASAPI), macOS (CoreAudio), Linux (ALSA)

## Repository Structure

```
precision-guitar-tuner/
├── external/                       # Git submodules
│   ├── kappa-core/                # OpenGL application framework (C++20)
│   ├── lib-guitar-io/             # Audio I/O abstraction (RtAudio wrapper)
│   └── lib-guitar-dsp/            # DSP algorithms (PFFFT, YIN, MPM)
├── src/
│   ├── main.cpp                   # Application entry point
│   ├── Application.cpp/.hpp       # kappa::Application subclass
│   ├── layers/                    # kappa-core Layer implementations
│   │   ├── AudioProcessingLayer   # Audio thread management
│   │   ├── TunerVisualizationLayer # Strobe/needle rendering
│   │   └── SettingsLayer          # UI controls
│   ├── tuner/                     # Tuner-specific logic
│   │   ├── StrobeTuner            # Strobe visualization
│   │   ├── NeedleTuner            # Needle display
│   │   └── SpectrumAnalyzer       # FFT visualization
│   └── utils/
│       ├── Config                 # Settings persistence
│       └── NoteConverter          # Frequency ↔ note conversion
├── include/tuner/                 # Public headers
├── assets/                        # Shaders, fonts, icons
├── tests/                         # Unit tests
├── CMakeLists.txt                 # Root build configuration
├── vcpkg.json                     # Dependency manifest
└── DEPENDENCIES.md                # Submodule version tracking
```

### Git Submodule Architecture

This project uses git submodules for code reuse across a planned 4-application guitar software suite:

- **lib-guitar-io**: Provides cross-platform audio device management and real-time I/O
  - Wraps RtAudio with RAII patterns
  - Handles device enumeration, hot-plug detection
  - Abstracts ASIO/CoreAudio/ALSA platform differences

- **lib-guitar-dsp**: Provides signal processing algorithms
  - PFFFT (BSD-licensed FFT, GPL-free)
  - YIN pitch detection (primary, 0.78% error rate)
  - MPM (McLeod Pitch Method, better for vibrato)
  - Note/frequency conversion utilities

- **kappa-core**: Application framework
  - Layer-based architecture for organizing rendering/logic
  - GLFW window management + OpenGL context
  - Type-safe event system (pub/sub)
  - spdlog integration with C++20 source locations

## Build System

### Initial Setup

```bash
# Initialize git submodules (REQUIRED on first clone)
git submodule update --init --recursive

# Install vcpkg dependencies (kappa-core requirements)
vcpkg install glfw3 glad glm spdlog
```

### Building

**Windows:**
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release

# Debug build
cmake --build build --config Debug
```

**macOS/Linux:**
```bash
# Release build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Debug build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Running

```bash
# Windows
.\build\Release\precision-tuner.exe

# macOS/Linux
./build/precision-tuner
```

### Running Tests

```bash
ctest --test-dir build --config Release --output-on-failure
```

### Updating Submodules

```bash
# Update all submodules to latest commits
git submodule update --remote --merge

# Update specific submodule
cd external/lib-guitar-io
git checkout v1.1.0  # or specific commit
cd ../..
git add external/lib-guitar-io
git commit -m "Update lib-guitar-io to v1.1.0"

# IMPORTANT: Always update DEPENDENCIES.md when changing submodule versions
```

## Architecture

### Layer-Based Design (kappa-core)

The application uses kappa-core's layer system to separate concerns:

```cpp
// Layers are processed in stack order
pushLayer(new AudioProcessingLayer());      // Bottom: audio thread
pushLayer(new TunerVisualizationLayer());   // Middle: rendering
pushLayer(new SettingsLayer());             // Top: UI controls
```

**Key Concepts:**
- Each layer has `onUpdate()` (logic) and `onRender()` (OpenGL drawing)
- Layers communicate via kappa-core's event bus (decoupled, type-safe)
- Audio processing runs on dedicated thread, publishes events to UI layers

### Audio Processing Pipeline

```
Audio Input (Rocksmith cable / USB interface)
    ↓
lib-guitar-io: RtAudio callback (audio thread)
    ↓
Circular buffer (lock-free, real-time safe)
    ↓
lib-guitar-dsp: YIN pitch detection
    ↓
Event published to UI thread
    ↓
TunerVisualizationLayer: Update strobe/needle display
```

### Thread Model

**CRITICAL:** Audio processing must be real-time safe:
- Audio callback runs on high-priority thread
- **NO allocations** in audio callback (no `new`, `std::vector::push_back`, etc.)
- **NO locks** in audio callback (use lock-free structures)
- **NO blocking I/O** in audio callback
- Use pre-allocated circular buffers for audio → UI communication

**Pattern:**
```cpp
// GOOD: Lock-free circular buffer
class AudioProcessingLayer : public kappa::Layer {
    LockFreeRingBuffer<float> audioBuffer;  // Pre-allocated

    void audioCallback(float* input, int frames) {
        // Real-time safe: only writes to ring buffer
        audioBuffer.write(input, frames);
    }

    void onUpdate() {
        // UI thread: reads from ring buffer and processes
        if (audioBuffer.available() >= ANALYSIS_WINDOW) {
            float* samples = audioBuffer.read(ANALYSIS_WINDOW);
            float pitch = pitchDetector.detect(samples);
            kappa::EventBus::publish(PitchDetectedEvent{pitch});
        }
    }
};
```

### Platform-Specific Audio APIs

The application automatically selects the best audio API per platform:

| Platform | Primary API | Fallback | Features |
|----------|-------------|----------|----------|
| Windows  | ASIO        | WASAPI   | 5-10ms latency with ASIO drivers |
| macOS    | CoreAudio   | -        | 2-5ms latency, multiple clients |
| Linux    | ALSA        | -        | 1-5ms latency (RT kernel recommended) |

**Platform Macros:**
- `PLATFORM_WINDOWS`: Windows-specific code
- `PLATFORM_MACOS`: macOS-specific code
- `PLATFORM_LINUX`: Linux-specific code

## Development Guidelines

### C++20 Standards

- Use **concepts** for template constraints
- Use **ranges** for expressive data processing
- Use **designated initializers** for clarity
- Use **std::span** instead of pointer+size pairs
- Leverage **source_location** for logging (kappa-core integrates this)

### Real-Time Audio Constraints

When working in audio callback context:

**✅ SAFE:**
- Reading/writing pre-allocated buffers
- Simple arithmetic operations
- Lock-free atomic operations (`std::atomic`)
- Stack-allocated variables

**❌ UNSAFE (will cause audio glitches):**
- `new` / `delete` / `malloc` / `free`
- `std::vector::push_back()`, `std::string` concatenation
- `std::mutex`, `std::lock_guard` (use lock-free structures)
- File I/O, network I/O
- `std::cout`, logging (use lock-free logging queue)

### Error Handling

```cpp
// Audio context: Return error codes, NO exceptions
AudioResult processAudio(float* buffer, int frames) {
    if (!buffer) return AudioResult::InvalidBuffer;
    if (frames <= 0) return AudioResult::InvalidFrameCount;
    // ... process
    return AudioResult::Success;
}

// UI context: Exceptions are acceptable
void loadSettings() {
    if (!configFile.exists()) {
        throw ConfigError("Config file not found");
    }
}
```

### Platform-Specific Code Isolation

```cpp
// audio/AudioEngine.cpp
#ifdef PLATFORM_WINDOWS
    rtAudio.openStream(..., RtAudio::WINDOWS_ASIO, ...);
#elif defined(PLATFORM_MACOS)
    rtAudio.openStream(..., RtAudio::MACOSX_CORE, ...);
#elif defined(PLATFORM_LINUX)
    rtAudio.openStream(..., RtAudio::LINUX_ALSA, ...);
#endif
```

## Development Principles

This codebase follows core software engineering principles:

### KISS (Keep It Simple, Stupid)
- Prefer straightforward implementations over clever solutions
- Each class has a single, clear responsibility (e.g., `AudioProcessingLayer` manages audio thread, `TunerVisualizationLayer` renders display)
- Avoid unnecessary abstractions - only abstract when multiple implementations exist or are anticipated
- Example: `NoteConverter` is a simple utility with static functions, not an elaborate type system

### YAGNI (You Aren't Gonna Need It)
- Don't implement features or abstractions until they're actually needed
- Avoid speculative generalization - extend when requirements emerge
- Example: Audio system started with basic chromatic tuning and expands only when new use cases appear (alternate tunings, polyphonic, etc.)
- Resist adding "just in case" parameters or configuration options

### SOLID Principles

**Single Responsibility Principle (SRP)**:
- Each class has one reason to change
- `AudioProcessingLayer` manages audio I/O and pitch detection, not rendering
- `TunerVisualizationLayer` handles OpenGL rendering, not audio processing
- `Config` manages settings persistence, not application logic

**Open/Closed Principle (OCP)**:
- Open for extension, closed for modification
- Add new tuning modes without modifying core tuner logic
- Add new visualization types via layer inheritance without changing base rendering
- Add new DSP algorithms in lib-guitar-dsp submodule without touching application code

**Liskov Substitution Principle (LSP)**:
- Derived classes are substitutable for base classes
- All `kappa::Layer` subclasses honor the lifecycle contract (`onAttach()`, `onUpdate()`, `onRender()`, `onDetach()`)
- All DSP algorithm implementations in lib-guitar-dsp follow the same interface contract (input buffer → output pitch/frequency)

**Interface Segregation Principle (ISP)**:
- Clients shouldn't depend on interfaces they don't use
- Layers only receive relevant events via `onEvent()`, not entire application state
- DSP algorithms expose minimal interface - just `detect()` or `process()`, not entire internal state
- Audio device interface exposes only necessary operations (start/stop/getDevices), not internal RtAudio complexity

**Dependency Inversion Principle (DIP)**:
- Depend on abstractions, not concretions
- Application depends on lib-guitar-io interface, not directly on RtAudio
- Layers depend on kappa::Event abstractions, not concrete event implementations
- DSP processing depends on abstract buffer interfaces, not specific audio formats

### DRY (Don't Repeat Yourself)
- Extract common logic into reusable functions/classes
- Audio buffer management utilities eliminate per-layer boilerplate
- Frequency/note conversion functions centralized in `NoteConverter`
- OpenGL shader loading extracted into utilities
- When you find yourself copying code, extract it into a shared utility or base class

**Anti-patterns to avoid**:
- Copy-pasting layer implementations - extract common base or helper functions
- Duplicating audio processing logic - centralize in lib-guitar-dsp submodule
- Repeating rendering code - use helper functions or rendering strategies
- Multiple device enumeration implementations - use lib-guitar-io abstraction

### Applying These Principles

When adding new features:
1. **Start simple** (KISS) - implement the minimum that works
2. **Don't anticipate** (YAGNI) - extend when you have concrete requirements
3. **Check responsibilities** (SRP) - if a class does multiple things, split it into layers
4. **Use abstractions** (OCP/DIP) - depend on interfaces (lib-guitar-io, lib-guitar-dsp), extend via inheritance/composition
5. **Look for duplication** (DRY) - extract common patterns before they spread

When reviewing code:
- Can this be simpler? (KISS)
- Is this solving a current problem or a hypothetical one? (YAGNI)
- Does this class have multiple reasons to change? (SRP)
- Are we modifying existing code instead of extending? (OCP)
- Would changing this break substitutability? (LSP)
- Are we forcing dependencies on unused functionality? (ISP)
- Are we depending on concrete implementations? (DIP)
- Have I seen this logic elsewhere? (DRY)

### Real-Time Audio Development Principles

**Additional principles specific to audio applications:**

1. **Determinism Over Flexibility**: In audio callback, predictable behavior beats configurability
   - Pre-allocate all buffers at initialization
   - Fixed-size data structures only
   - No dynamic dispatch in critical path

2. **Separation of Real-Time and Non-Real-Time**: Strict thread separation
   - Audio thread: only lock-free operations, pre-allocated buffers
   - UI thread: can allocate, use locks, perform I/O
   - Communication: lock-free ring buffers or atomic flags only

3. **Fail Fast in Development, Graceful in Production**:
   - Development: Assert on contract violations (buffer overruns, invalid states)
   - Production: Return error codes, log issues, maintain audio continuity

4. **Measure, Don't Assume**: Audio performance requires profiling
   - Profile callback timing regularly
   - Verify lock-free claim with thread sanitizers
   - Test on low-end hardware (not just development machine)

## Key Technical Concepts

### YIN vs MPM Pitch Detection

**Use YIN (primary algorithm):**
- Standard tuning scenarios
- Best overall accuracy (0.78% error rate)
- Fast (optimized FastYIN with FFT)
- Robust to noise

**Use MPM (fallback):**
- Vibrato detection (better dynamic pitch tracking)
- Smaller analysis windows (better for changing pitch)
- When YIN confidence is low (<0.8)

```cpp
// Hybrid approach
float pitch = yinDetector.detect(buffer, size);
float confidence = yinDetector.getConfidence();

if (confidence < 0.8f) {
    // Low confidence, try MPM
    float mpmPitch = mpmDetector.detect(buffer, size);
    pitch = weightedAverage(pitch, mpmPitch, confidence);
}
```

### Achieving ±0.1 Cent Accuracy

**Requirements:**
1. **48kHz sample rate** (interface native rate, better than 44.1kHz)
2. **YIN interpolation** (sub-sample accuracy)
3. **Interface clock sync** (use audio interface crystal, not OS clock)
4. **Calibrated reference** (user-adjustable A=430-450Hz)

**Accuracy calculation:**
- At A4 (440 Hz): 1 cent = 0.254 Hz, 0.1 cent = 0.0254 Hz
- YIN interpolation achieves <0.01 Hz accuracy
- Display precision: Show to 0.1 cent (e.g., "+2.3 cents")

### Rocksmith Realtone Cable Support

**Auto-detection pattern:**
```cpp
bool isRocksmithCable(const RtAudio::DeviceInfo& info) {
    return info.name.find("Rocksmith") != std::string::npos ||
           info.name.find("Guitar Adapter") != std::string::npos;
}

// Prioritize Rocksmith cable in device list
int selectDefaultDevice() {
    // First pass: look for Rocksmith cable
    for (auto& device : audioDevices) {
        if (isRocksmithCable(device.info)) return device.id;
    }
    // Second pass: any input device
    // ...
}
```

**Known issues:**
- Rocksmith cable is USB 2.0 only (may fail on USB 3.0 ports)
- Recommend USB 2.0 hub if detection fails
- 16-bit ADC (vs 24-bit pro interfaces), but sufficient for tuning

## Common Development Tasks

### Adding a New Tuning Mode

1. Add tuning to `src/tuner/TuningPresets.cpp`:
```cpp
TuningPreset TuningPresets::DropD = {
    .name = "Drop D",
    .notes = {
        Note::D2,  // Low D (146.83 Hz)
        Note::A2,  // A (110.00 Hz)
        Note::D3,  // D (146.83 Hz)
        Note::G3,  // G (196.00 Hz)
        Note::B3,  // B (246.94 Hz)
        Note::E4   // High E (329.63 Hz)
    }
};
```

2. Update UI dropdown in `src/layers/SettingsLayer.cpp`

### Creating a New kappa-core Layer

```cpp
// include/tuner/MyCustomLayer.hpp
#pragma once
#include <kappa/Layer.hpp>

class MyCustomLayer : public kappa::Layer {
public:
    void onAttach() override;    // Initialization
    void onDetach() override;    // Cleanup
    void onUpdate() override;    // Per-frame logic
    void onRender() override;    // OpenGL rendering
    void onEvent(kappa::Event& e) override;  // Event handling
};
```

### Integrating a New DSP Algorithm

1. Add algorithm to **lib-guitar-dsp** submodule (shared across apps):
```cpp
// external/lib-guitar-dsp/include/guitar-dsp/MyAlgorithm.hpp
namespace GuitarDSP {
    class MyAlgorithm {
    public:
        float process(const float* input, int size);
    };
}
```

2. Use in application:
```cpp
#include <guitar-dsp/MyAlgorithm.hpp>

void AudioProcessingLayer::onUpdate() {
    GuitarDSP::MyAlgorithm algo;
    float result = algo.process(samples, sampleCount);
}
```

### Testing Audio Processing Components

```cpp
// tests/TestYIN.cpp
#include <gtest/gtest.h>
#include <guitar-dsp/YIN.hpp>

TEST(YINTest, DetectsA440) {
    // Generate pure 440 Hz sine wave
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 2048;
    float buffer[bufferSize];

    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] = std::sin(2.0f * M_PI * 440.0f * i / sampleRate);
    }

    GuitarDSP::YIN yin(sampleRate);
    float detectedPitch = yin.detect(buffer, bufferSize);

    // Should detect within 0.5 Hz (generous for test)
    EXPECT_NEAR(detectedPitch, 440.0f, 0.5f);
}
```

### Debugging Audio Issues

**Enable verbose audio logging:**
```cpp
// main.cpp
#define RTAUDIO_DEBUG  // Before including RtAudio
#include <RtAudio.h>
```

**Common issues:**
- **Crackling/dropouts**: Buffer size too small, increase to 128-256 samples
- **High latency**: Check if ASIO drivers installed (Windows)
- **No input detected**: Check device permissions (macOS/Linux)
- **Wrong device selected**: Verify with `RtAudio::getDeviceInfo()`

**Profiling audio callback:**
```cpp
void audioCallback(float* input, int frames) {
    auto start = std::chrono::high_resolution_clock::now();

    // ... process audio ...

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Audio callback must finish in < buffer_size / sample_rate
    // For 128 samples @ 48kHz = 2666 microseconds max
    if (duration.count() > 2000) {
        LOG_WARN("Audio callback took {}us (too slow!)", duration.count());
    }
}
```

## Dependencies and Version Management

Always update **DEPENDENCIES.md** when changing submodule versions:

```markdown
# DEPENDENCIES.md

| Component | Repository | Version/Commit | Date Pinned |
|-----------|------------|----------------|-------------|
| lib-guitar-io | github.com/yourorg/lib-guitar-io | v1.0.0 (a1b2c3d) | 2025-01-15 |
| lib-guitar-dsp | github.com/yourorg/lib-guitar-dsp | v1.0.0 (e4f5g6h) | 2025-01-15 |
| kappa-core | github.com/Konstantysz/kappa-core | commit i7j8k9l | 2025-01-15 |
```

Check `git submodule status` frequently to verify pinned versions.

## Performance Targets

- **Accuracy:** ±0.1 cent (competitive with Peterson StroboSoft)
- **Latency:** <10ms end-to-end (input → detection → display)
- **Update Rate:** 60 Hz UI refresh
- **CPU Usage:** <5% on modern hardware (single core)
- **Memory:** <50 MB resident (no leaks in long-running sessions)

## Licensing

- **Application Code:** MIT License (commercial use allowed)
- **lib-guitar-io:** MIT License
- **lib-guitar-dsp:** BSD License (PFFFT), MIT (YIN/MPM)
- **kappa-core:** MIT License
- **RtAudio:** MIT-like permissive license

All dependencies are GPL-free for commercial distribution.
