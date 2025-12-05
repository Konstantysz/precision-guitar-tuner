# Changelog

All notable changes to Precision Guitar Tuner are documented here.

Format based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-12-06

First stable release. Professional-grade guitar tuner with studio-quality pitch detection.

### Added

- Hybrid YIN/MPM pitch detection algorithm with automatic fallback (±0.1 cent accuracy)
- Harmonic rejection for 2x, 3x, 4x overtones
- Cross-platform audio support: Windows (ASIO/WASAPI), macOS (CoreAudio), Linux (ALSA)
- Multiple tuning modes: Standard, Drop D, Drop C, DADGAD, Open G, Open D, Chromatic
- Premium retro gauge visualization with realistic wood/chrome/glass rendering
- Reference tone generator (100-1000 Hz adjustable)
- Input monitoring (digital amp functionality)
- Drone mode (continuous reference playback)
- Polyphonic mode (simultaneous 6-string playback)
- In-tune beep confirmation
- Separate input/output device selection
- 11 keyboard shortcuts for hands-free operation (Space, D, P, R, B, M, Up/Down, Ctrl+Comma, Esc, F11, F1)
- Interactive tooltips on all 13 settings controls
- Built-in help system (Quick Start Guide, User Guide, About dialog)
- Keyboard shortcuts overlay (F1)
- Configuration persistence across sessions
- Responsive window sizing with layout preservation
- External GLSL shader system (geometry.vert/frag, text.vert/frag)
- TrueType font rendering via stb_truetype
- 66 unit tests using Google Test framework
- Automated CI/CD for Windows/macOS/Linux builds
- Static analysis integration (clang-tidy, cppcheck)
- Code coverage tracking
- CPack-based packaging (ZIP/DMG/TGZ)

### Technical

- C++20 codebase with concepts, ranges, designated initializers, std::span
- OpenGL 3.3+ rendering pipeline
- Real-time safe audio: lock-free algorithms, zero-allocation callbacks, pre-allocated buffers
- 60 FPS UI refresh rate
- GPL-free dependencies (MIT/BSD licensing)
- Sub-10ms audio latency on professional interfaces
- CPU usage <5% single core, memory footprint <50MB
