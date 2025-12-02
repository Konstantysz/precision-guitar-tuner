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
│   ├── lib-guitar-dsp/            # DSP algorithms (PFFFT, YIN, MPM)
│   └── stb/                       # stb_truetype single-header library
├── src/
│   ├── PrecisionGuitarTuner.cpp   # Application entry point
│   ├── Layers/                    # kappa-core Layer implementations
│   │   ├── AudioProcessingLayer.h/.cpp      # Audio thread management
│   │   └── TunerVisualizationLayer.h/.cpp   # Visual tuner rendering
│   └── FontRenderer.h/.cpp        # TrueType font rendering (stb_truetype)
├── assets/shaders/                # GLSL shader files
├── tests/                         # Unit tests (Google Test)
├── CMakeLists.txt                 # Root build configuration
├── vcpkg.json                     # Dependency manifest
└── DEPENDENCIES.md                # Submodule version tracking
```

**Key Directories:**

- `src/Layers/`: All layer classes are in the `PrecisionTuner::Layers` namespace
- `assets/shaders/`: External GLSL shader files (copied to build directory automatically)

### Git Submodule Architecture

This project uses git submodules for code reuse across a planned 4-application guitar software suite:

- **lib-guitar-io**: Cross-platform audio device management and real-time I/O
  - Wraps RtAudio with RAII patterns, uses `std::span` for type-safe audio buffers
  - Handles device enumeration, hot-plug detection
  - Abstracts ASIO/CoreAudio/ALSA platform differences

- **lib-guitar-dsp**: Signal processing algorithms
  - PFFFT (BSD-licensed FFT, GPL-free), YIN pitch detection, MPM (McLeod Pitch Method)
  - Uses `std::span` for type-safe buffer interfaces

- **kappa-core**: Application framework
  - Layer-based architecture, GLFW window management + OpenGL context
  - Type-safe event system (pub/sub), spdlog integration

## Build System

### Initial Setup

```bash
# Initialize git submodules (REQUIRED on first clone)
git submodule update --init --recursive

# Install vcpkg dependencies
vcpkg install glfw3 glad glm spdlog
```

### Building

**Windows:**

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

**macOS/Linux:**

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Running:**

```bash
# Windows
.\build\Release\precision-guitar-tuner.exe

# macOS/Linux
./build/precision-guitar-tuner
```

### Release Process

Automated releases via GitHub Actions (`deploy.yml`):

1. Tag a release: `git tag v1.0.0 && git push origin v1.0.0`
2. Pipeline builds for Windows, macOS, and Linux
3. Packages artifacts (ZIP, DMG, TGZ) via CPack
4. Creates GitHub Release and uploads artifacts

### Testing

```bash
# Build and run all tests
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Updating Submodules

```bash
# Update all submodules
git submodule update --remote --merge

# Update specific submodule
cd external/lib-guitar-io && git checkout v1.1.0 && cd ../..
git add external/lib-guitar-io
git commit -m "Update lib-guitar-io to v1.1.0"

# IMPORTANT: Always update DEPENDENCIES.md when changing submodule versions
```

## Architecture

### Layer-Based Design (kappa-core)

```cpp
// Layers are processed in stack order
pushLayer(new AudioProcessingLayer());      // Bottom: audio thread
pushLayer(new TunerVisualizationLayer());   // Middle: rendering
pushLayer(new SettingsLayer());             // Top: UI controls
```

- Each layer has `onUpdate()` (logic) and `onRender()` (OpenGL drawing)
- Layers communicate via kappa-core's event bus (decoupled, type-safe)

### Audio Processing Pipeline

```
Audio Input (Rocksmith cable / USB interface)
    ↓
lib-guitar-io: RtAudio callback (audio thread) → std::span wrapper
    ↓
AudioProcessingLayer: std::span<const float>
    ↓
lib-guitar-dsp: YIN pitch detection (std::span interface)
    ↓
Event published to UI thread
    ↓
TunerVisualizationLayer: Update needle display
```

### Thread Model

**CRITICAL:** Audio processing must be real-time safe:

- Audio callback runs on high-priority thread
- **NO allocations** in audio callback (no `new`, `std::vector::push_back`, etc.)
- **NO locks** in audio callback (use lock-free structures)
- **NO blocking I/O** in audio callback
- Use pre-allocated circular buffers for audio → UI communication

**Example Pattern:**

```cpp
// Lock-free circular buffer
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

| Platform | Primary API | Fallback | Features |
|----------|-------------|----------|----------|
| Windows  | ASIO        | WASAPI   | 5-10ms latency with ASIO drivers |
| macOS    | CoreAudio   | -        | 2-5ms latency, multiple clients |
| Linux    | ALSA        | -        | 1-5ms latency (RT kernel recommended) |

**Platform Macros:** `PLATFORM_WINDOWS`, `PLATFORM_MACOS`, `PLATFORM_LINUX`

**Example:**

```cpp
#ifdef PLATFORM_WINDOWS
    rtAudio.openStream(..., RtAudio::WINDOWS_ASIO, ...);
#elif defined(PLATFORM_MACOS)
    rtAudio.openStream(..., RtAudio::MACOSX_CORE, ...);
#elif defined(PLATFORM_LINUX)
    rtAudio.openStream(..., RtAudio::LINUX_ALSA, ...);
#endif
```

## Development Guidelines

### Coding Standards

**Naming Conventions:**

- **PascalCase** for classes, structs, and functions
  - Examples: `AudioProcessingLayer`, `GetLatestPitch()`, `OnUpdate()`
- **camelCase** for ALL variables (including class member variables)
  - **NO trailing underscores** on member variables
  - Examples: `config`, `audioDevice`, `pitchDetector`, `latestFrequency`

**Code Formatting:**

- **Line limit:** 120 characters
- **Indentation:** 4 spaces (no tabs)
- **Brace style:** Allman (opening braces on new lines)
- **Pointer alignment:** Right-aligned (`Type *ptr`, not `Type* ptr`)
- **Include ordering:** Automatically sorted by clang-format with priority system

**Code Organization:**

- **Function ordering:** Function definitions in `.cpp` files should match the declaration order in corresponding `.h` files
  - Makes navigation between header/source easier, improves code review experience

**Formatting Tools:**

- `.clang-format` - C++20 code formatting (120 char limit, Allman braces)
- `.clang-tidy` - Static analysis with C++20 modernize checks
- `.cmake-format` - CMake file formatting
- `.pre-commit-config.yaml` - Automated formatting on commit

Run `clang-format -i <file>` to format individual files.

### C++20 Standards

- Use **concepts** for template constraints
- Use **ranges** for expressive data processing
- Use **designated initializers** for clarity
- Use **std::span** instead of pointer+size pairs
- Leverage **source_location** for logging (kappa-core integrates this)

### Real-Time Audio Constraints

**✅ SAFE in audio callback:**

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

## Development Principles

This codebase follows core software engineering principles. For detailed explanations, see external resources on [SOLID](https://en.wikipedia.org/wiki/SOLID), [KISS](https://en.wikipedia.org/wiki/KISS_principle), [YAGNI](https://en.wikipedia.org/wiki/You_aren%27t_gonna_need_it), and [DRY](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself).

### Quick Reference

**KISS (Keep It Simple, Stupid):**

- Prefer straightforward implementations over clever solutions
- Each class has a single, clear responsibility
- Avoid unnecessary abstractions - only abstract when multiple implementations exist or are anticipated

**YAGNI (You Aren't Gonna Need It):**

- Don't implement features or abstractions until they're actually needed
- Avoid speculative generalization - extend when requirements emerge
- Resist adding "just in case" parameters or configuration options

**SOLID Principles:**

- **SRP:** Each class has one reason to change (e.g., `AudioProcessingLayer` manages audio I/O, not rendering)
- **OCP:** Open for extension, closed for modification (add new tuning modes without modifying core tuner logic)
- **LSP:** Derived classes are substitutable for base classes (all `kappa::Layer` subclasses honor the lifecycle contract)
- **ISP:** Clients shouldn't depend on interfaces they don't use (layers only receive relevant events)
- **DIP:** Depend on abstractions, not concretions (application depends on lib-guitar-io interface, not directly on RtAudio)

**DRY (Don't Repeat Yourself):**

- Extract common logic into reusable functions/classes
- When you find yourself copying code, extract it into a shared utility or base class

### Applying These Principles

**When adding new features:**

1. Start simple (KISS) - implement the minimum that works
2. Don't anticipate (YAGNI) - extend when you have concrete requirements
3. Check responsibilities (SRP) - if a class does multiple things, split it into layers
4. Use abstractions (OCP/DIP) - depend on interfaces (lib-guitar-io, lib-guitar-dsp), extend via inheritance/composition
5. Look for duplication (DRY) - extract common patterns before they spread

**When reviewing code:**

- Can this be simpler? (KISS)
- Is this solving a current problem or a hypothetical one? (YAGNI)
- Does this class have multiple reasons to change? (SRP)
- Are we modifying existing code instead of extending? (OCP)
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

This application uses a **HybridPitchDetector** that combines both algorithms for optimal accuracy.

**YIN (primary algorithm):**

- Best overall accuracy (0.78% error rate), fast (optimized FastYIN with FFT)
- Fine-tuned parameters for guitar: threshold=0.10, range=80-1200 Hz

**MPM (fallback algorithm):**

- Better for vibrato detection (NSDF algorithm), activated when YIN confidence < 0.8
- Smaller analysis windows, higher threshold (0.93) for precise detection

**Harmonic Rejection:**

- Detects 2x, 3x, 4x harmonics within 5% tolerance
- Plausible guitar fundamental range: 80-400 Hz
- Example: 164.8 Hz (2x error) → 82.4 Hz (Low E)

### Pitch Stabilization

**Hybrid (recommended for guitar):**

- Combines median filter (spike rejection) + confidence-weighted EMA
- High confidence → faster convergence; Low confidence → more smoothing

**Configuration:**

```cpp
AudioProcessingLayer::Config config;
config.stabilizerType = AudioProcessingLayer::Config::StabilizerType::Hybrid;
config.emaAlpha = 0.3f;          // Higher = more responsive
config.medianWindowSize = 5;      // Larger = more smoothing
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

## Common Development Tasks

### Adding a New Tuning Mode

1. Add tuning to [TuningPresets.cpp](src/tuner/TuningPresets.cpp):

```cpp
TuningPreset TuningPresets::DropD = {
    .name = "Drop D",
    .notes = {
        Note::D2,  // Low D (146.83 Hz)
        Note::A2, Note::D3, Note::G3, Note::B3, Note::E4
    }
};
```

2. Update UI dropdown in [SettingsLayer.cpp](src/layers/SettingsLayer.cpp)

### Debugging Audio Issues

**Enable verbose audio logging:**

```cpp
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

Always update [DEPENDENCIES.md](DEPENDENCIES.md) when changing submodule versions:

```markdown
| Component | Repository | Version/Commit | Date Pinned |
|-----------|------------|----------------|-------------|
| lib-guitar-io | github.com/yourorg/lib-guitar-io | v0.0.1 (a1b2c3d) | 2025-01-15 |
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
