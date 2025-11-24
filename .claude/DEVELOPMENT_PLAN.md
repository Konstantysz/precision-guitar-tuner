# Precision Guitar Tuner - Development Plan

## Project Vision

Build a professional-grade desktop guitar tuner application with ±0.1 cent accuracy, native audio interface support, and cross-platform compatibility (Windows, macOS, Linux). Position as an affordable alternative to Peterson StroboSoft ($50-100) at $39.99.

## Technology Stack

- **Language:** C++20
- **UI Framework:** kappa-core (OpenGL-based, MIT license)
- **Audio I/O:** RtAudio (via lib-guitar-io submodule)
- **DSP:** PFFFT + YIN/MPM pitch detection (via lib-guitar-dsp submodule)
- **Build System:** CMake 3.20+ with vcpkg
- **Version Control:** Git with submodules

## Architecture Overview

```
Application (kappa-core)
    ├── AudioProcessingLayer (audio thread management)
    ├── TunerVisualizationLayer (OpenGL rendering)
    └── SettingsLayer (UI controls)

Dependencies (Git Submodules)
    ├── lib-guitar-io (RtAudio wrapper, device management)
    ├── lib-guitar-dsp (PFFFT, YIN, MPM algorithms)
    └── kappa-core (application framework)
```

## Development Phases

### Phase 0: Repository Setup (Week 1) ✓ IN PROGRESS

**Goals:**
- Initialize git repository
- Set up git submodules structure
- Create CMake build configuration
- Establish project documentation

**Tasks:**
1. [x] Create CLAUDE.md documentation
2. [ ] Initialize git repository
3. [ ] Create submodule repositories:
   - [ ] lib-guitar-io
   - [ ] lib-guitar-dsp
4. [ ] Add kappa-core as submodule
5. [ ] Create root CMakeLists.txt
6. [ ] Set up vcpkg.json for dependencies
7. [ ] Create DEPENDENCIES.md for version tracking
8. [ ] Create .gitignore
9. [ ] Create README.md

**Deliverables:**
- Buildable skeleton project (empty main.cpp)
- All submodules initialized
- CMake successfully configures on Windows

---

### Phase 1: Core Audio Infrastructure (Weeks 2-3)

**Goals:**
- Implement lib-guitar-io submodule (RtAudio wrapper)
- Audio device enumeration and selection
- Real-time audio input capture
- Lock-free circular buffer for thread communication

**Tasks:**

#### lib-guitar-io Submodule
1. [ ] Create AudioDevice class (RAII wrapper for RtAudio device info)
2. [ ] Create AudioEngine class (manages RtAudio instance)
3. [ ] Implement device enumeration
4. [ ] Implement device hot-plug detection
5. [ ] Create platform-specific audio stream initialization (ASIO/CoreAudio/ALSA)
6. [ ] Implement audio callback with circular buffer
7. [ ] Add error handling and logging
8. [ ] Write unit tests for device management

#### Application Integration
9. [ ] Create AudioProcessingLayer (kappa-core layer)
10. [ ] Integrate lib-guitar-io into application
11. [ ] Implement Rocksmith cable auto-detection
12. [ ] Add device selection persistence (Config class)
13. [ ] Test audio input on Windows (ASIO/WASAPI)

**Deliverables:**
- lib-guitar-io library (reusable across future apps)
- Working audio input capture
- Device selection UI (basic dropdown)
- Audio flowing into circular buffer at 48kHz

**Success Criteria:**
- Audio input visible in debug logs
- Latency < 10ms on ASIO
- No buffer overruns or underruns
- Hot-plug detection working

---

### Phase 2: DSP Foundation (Week 3-4)

**Goals:**
- Implement lib-guitar-dsp submodule
- YIN pitch detection algorithm
- Frequency/note conversion utilities
- Real-time pitch detection from audio buffer

**Tasks:**

#### lib-guitar-dsp Submodule
1. [ ] Integrate PFFFT library (BSD license)
2. [ ] Create FFT wrapper class
3. [ ] Implement YIN pitch detection algorithm
4. [ ] Add YIN confidence calculation
5. [ ] Implement MPM algorithm (fallback)
6. [ ] Create NoteConverter utility (frequency ↔ note/cents)
7. [ ] Write comprehensive unit tests (test with generated sine waves)
8. [ ] Optimize for real-time performance (<5ms processing)

#### Application Integration
9. [ ] Integrate lib-guitar-dsp into AudioProcessingLayer
10. [ ] Implement pitch detection pipeline (buffer → YIN → frequency)
11. [ ] Create PitchDetectedEvent for event bus
12. [ ] Add smoothing/filtering for stable readings
13. [ ] Test accuracy with known frequencies (A440 tuning fork app)

**Deliverables:**
- lib-guitar-dsp library with YIN and MPM
- Real-time pitch detection working
- Accuracy within ±0.5 Hz for test signals
- Event bus publishing pitch data to UI

**Success Criteria:**
- Detects A440 within ±0.1 Hz
- Processing time < 5ms per buffer
- Stable readings (not jumping between notes)
- YIN confidence metric reliable

---

### Phase 3: Enhanced UI and Features (Weeks 4-5) ✅ PARTIALLY COMPLETE

**Status:** 5 of 7 major features complete, 2 in progress (71%)
**Completed:** External GLSL shaders ✅, TrueType font rendering ✅, Configuration persistence ✅, Responsive layout ✅, Settings layer (basic UI) ✅
**In Progress:** Device selection controls, Tuning modes
**Deferred:** Spectrum analyzer (optional)

**Goals:**

- ✅ Externalize shaders to .glsl files (COMPLETE)
- ✅ Implement TrueType font rendering (COMPLETE)
- ✅ Configuration persistence (JSON save/load) (COMPLETE)
- ✅ Responsive window sizing and layout (COMPLETE)
- Settings layer with device selection
- Multiple tuning modes (chromatic, standard, drop D, etc.)
- Optional: Spectrum analyzer visualization

**Tasks:**

#### Rendering Layer ✅ COMPLETE

1. [x] Create TunerVisualizationLayer (kappa-core layer)
2. [x] Implement needle meter rendering (OpenGL)
3. [x] Add note name display (current detected note)
4. [x] Add frequency display (Hz)
5. [x] Add cents offset display (±50 cents range)
6. [x] Create "in-tune" indicator (green zone)
7. [x] Design OpenGL shaders for smooth needle animation
8. [x] **Externalize shaders to .glsl files** ✅ COMPLETE
   - `assets/shaders/geometry.vert/.frag` - Shape rendering
   - `assets/shaders/text.vert/.frag` - Text rendering with texture sampling
   - CMake POST_BUILD command auto-copies to build directory
9. [x] Implement reference pitch line (target frequency)
10. [x] **TrueType font rendering with stb_truetype** ✅ COMPLETE
    - FontRenderer.h/.cpp implementation
    - Font atlas generation (512x512 texture)
    - System font loading (Arial, Consola, Helvetica, DejaVu Sans)
    - Antialiased text rendering
    - Text width calculation for centering

#### Configuration System ✅ COMPLETE

11. [x] Create Config.h/.cpp class for application settings
12. [x] Implement JSON serialization with nlohmann-json (already in vcpkg)
13. [x] Save/load: audio device ID, reference pitch, tuning mode, window state
14. [x] Config file location: user directory (platform-specific paths)
    - Windows: `%APPDATA%/PrecisionTuner/config.json`
    - macOS: `~/Library/Application Support/PrecisionTuner/config.json`
    - Linux: `~/.config/PrecisionTuner/config.json`
15. [x] Integration in PrecisionGuitarTuner.cpp (load on startup, save on shutdown)

#### Responsive Layout ✅ COMPLETE
16. [x] ~~Override OnEvent() in TunerVisualizationLayer to handle WindowResizeEvent~~ (used GLFW direct query instead)
17. [x] Dynamic glViewport() updates on window resize
18. [x] Aspect-ratio-aware coordinate scaling (ScaleToAspectRatio() helper method added)
19. [x] Window state persistence via Config system (width/height saved to config.json)
20. [x] Window size constraints (400×300 minimum, 3840×2160 maximum)

#### Settings UI (Infrastructure Ready)
21. [ ] Create SettingsLayer (kappa-core layer)
22. [ ] Choose UI framework: ImGui (recommended) or custom OpenGL controls
23. [ ] Add device selection dropdown (uses AudioProcessingLayer::GetAvailableDevices())
24. [ ] Add reference pitch adjustment slider (A=430-450 Hz)
25. [ ] Add tuning mode selector (chromatic, standard, drop D, etc.)
26. [ ] Runtime device switching (stop current, start new)
27. [ ] Add SettingsLayer to application layer stack

#### Multiple Tuning Modes (NEW)
28. [ ] Create TuningPresets.h/.cpp class
29. [ ] Implement tuning preset system with note/frequency arrays
30. [ ] Add tuning presets: Standard (EADGBE), Drop D, Drop C, DADGAD, Open G, etc.
31. [ ] Add chromatic mode (current behavior - any note)
32. [ ] Visual indicator for "closest target string" in standard tuning modes
33. [ ] Tuning mode selector UI in SettingsLayer
34. [ ] Persist selected tuning mode in Config

#### Optional: Spectrum Analyzer
35. [ ] FFT visualization using existing PFFFT from lib-guitar-dsp
36. [ ] Frequency magnitude spectrum bar graph
37. [ ] Harmonic peak highlighting
38. [ ] Toggle between tuner/spectrum modes

#### Application Polish ✅ COMPLETE
38. [x] Create Application class (kappa::Application subclass)
39. [x] Set up layer stack (Audio → Visualization)
40. [x] Implement window title and icon
41. [ ] Add SettingsLayer to stack (pending implementation)

**Deliverables:**
- ✅ Functional tuner UI with modern OpenGL rendering
- ✅ External GLSL shader files
- ✅ TrueType font rendering for text display
- ✅ 60 FPS rendering with color-coded tuning feedback
- ✅ Configuration persistence (JSON save/load with platform paths)
- ✅ Responsive window layout with size constraints
- [ ] Settings layer with device selection
- [ ] Multiple tuning modes

**Success Criteria:**
- ✅ Needle responds in real-time (<50ms visual latency)
- ✅ UI is clear and readable
- ✅ No UI lag or stuttering at 60 FPS
- ✅ Settings persist between sessions (config save/load working)
- ✅ Window resize handled gracefully (400×300 min, 3840×2160 max)
- [ ] Device switching works without restart

---

### Phase 4: Advanced Tuning Features (Week 6)

**Goals:**
- Multiple tuning modes (standard, drop, alternate)
- Chromatic mode
- Audio feedback (in-tune tone, reference pitch playback)
- Visual feedback improvements
- Accuracy refinement

**Tasks:**

#### Tuning System
1. [ ] Create TuningPresets class
2. [ ] Implement standard tuning (EADGBE)
3. [ ] Implement drop tunings (Drop D, Drop C, Drop B)
4. [ ] Implement alternate tunings (DADGAD, Open G, Open D)
5. [ ] Add chromatic mode (any note detection)
6. [ ] Create tuning preset selector UI
7. [ ] Implement "closest string" indicator (for standard tuning)

#### Audio Feedback (NEW)
8. [ ] Implement tone generator in lib-guitar-io (sine wave synthesis)
9. [ ] Add "in-tune beep" feature (configurable on/off)
10. [ ] Implement reference pitch playback (play target note)
11. [ ] Add tone volume control in settings
12. [ ] Create audio mixing (input + tone output)
13. [ ] Add "drone mode" (continuous reference tone while tuning)
14. [ ] Implement polyphonic tone generation (play full chord for checking tuning)

#### Visualization Enhancements
15. [ ] Add frequency spectrum display
16. [ ] Improve needle smoothing (exponential moving average)
17. [ ] Add color coding (red far, yellow close, green in-tune)
18. [ ] Implement cents history graph (optional)
19. [ ] Add audio level meter (input gain indicator)

#### Accuracy Improvements
20. [ ] Fine-tune YIN parameters for guitar frequencies
21. [ ] Implement hybrid YIN/MPM approach (use MPM when YIN confidence < 0.8)
22. [ ] Add harmonic rejection (ignore overtones)
23. [ ] Calibrate for different string gauges

**Deliverables:**
- 10+ tuning presets available
- Chromatic mode working
- Audio feedback (beep on in-tune, reference tone playback)
- Enhanced visual feedback
- ±0.1 cent accuracy achieved

**Success Criteria:**
- All tuning modes detect correctly
- Audio feedback is clear and non-intrusive
- Reference tones are accurate to detected reference pitch
- Visual feedback is intuitive
- Accuracy verified with oscilloscope
- Low E string (82.4 Hz) detects reliably

---

### Phase 5: Strobe Tuner (Week 7)

**Goals:**
- Implement strobe tuner visualization
- Sub-cent precision feedback
- Professional-grade visual tuning

**Tasks:**

#### Strobe Rendering
1. [ ] Research Peterson strobe tuner algorithm
2. [ ] Create strobe shader (rotating disc pattern)
3. [ ] Implement strobe speed calculation (based on cents offset)
4. [ ] Add strobe direction indicator (sharp vs flat)
5. [ ] Create strobe color scheme
6. [ ] Optimize shader performance

#### Strobe Integration
7. [ ] Add strobe mode toggle in settings
8. [ ] Implement smooth transition (needle ↔ strobe)
9. [ ] Add strobe sensitivity adjustment
10. [ ] Create strobe calibration mode

**Deliverables:**
- Working strobe tuner mode
- Visual precision at 0.1 cent level
- Toggle between needle and strobe

**Success Criteria:**
- Strobe stops completely when in-tune (±0.1 cent)
- Direction clearly indicates sharp/flat
- Performance maintains 60 FPS
- Intuitive for professional users

---

### Phase 6: Cross-Platform Build and Testing (Week 8)

**Goals:**
- Build on macOS
- Build on Linux
- Platform-specific testing
- CI/CD pipeline

**Tasks:**

#### macOS Support
1. [ ] Test CMake build on macOS
2. [ ] Verify CoreAudio integration
3. [ ] Test with built-in microphone and USB interface
4. [ ] Create .dmg installer (optional)

#### Linux Support
5. [ ] Test CMake build on Ubuntu 22.04
6. [ ] Verify ALSA integration
7. [ ] Test with USB audio interface
8. [ ] Create AppImage package (optional)

#### CI/CD
9. [ ] Create GitHub Actions workflow
10. [ ] Configure multi-platform builds (Windows, macOS, Linux)
11. [ ] Add automated testing (unit tests, integration tests)
12. [ ] Set up release artifacts

#### Testing
13. [ ] Write integration tests
14. [ ] Test with multiple audio interfaces
15. [ ] Test Rocksmith cable on all platforms
16. [ ] Performance profiling on low-end hardware
17. [ ] Memory leak detection (Valgrind, ASAN)

**Deliverables:**
- Working builds on Windows, macOS, Linux
- CI/CD pipeline functional
- Test coverage > 70%
- Installers for all platforms

**Success Criteria:**
- All platforms achieve <10ms latency
- No memory leaks detected
- CI builds passing
- Ready for beta release

---

### Phase 7: Beta Testing and Polish (Weeks 9-10)

**Goals:**
- Beta program with 50-100 users
- Bug fixes based on feedback
- Performance optimization
- Documentation

**Tasks:**

#### Beta Program
1. [ ] Recruit beta testers (Gearspace, Reddit)
2. [ ] Create feedback form (Google Forms)
3. [ ] Distribute beta builds
4. [ ] Collect and triage feedback
5. [ ] Fix critical bugs
6. [ ] Implement high-value feature requests

#### Polish
7. [ ] Improve error messages
8. [ ] Add tooltips and help text
9. [ ] Optimize startup time
10. [ ] Reduce CPU usage
11. [ ] Improve UI aesthetics
12. [ ] Add keyboard shortcuts

#### Documentation
13. [ ] Write user manual
14. [ ] Create quick start guide
15. [ ] Add troubleshooting section (USB 3.0 issues, driver installation)
16. [ ] Create video tutorial (YouTube)
17. [ ] Write developer documentation

**Deliverables:**
- Beta-tested application
- Comprehensive user documentation
- Demo video
- Testimonials from beta users

**Success Criteria:**
- Beta satisfaction > 80%
- All critical bugs fixed
- Documentation complete
- Ready for launch

---

### Phase 8: Launch Preparation (Week 11)

**Goals:**
- Marketing materials
- Pricing finalized
- Distribution setup
- Launch strategy

**Tasks:**

#### Marketing
1. [ ] Create landing page
2. [ ] Write comparison blog post (vs Peterson, vs mobile apps)
3. [ ] Create YouTube demo video
4. [ ] Prepare forum announcements (Gearspace, Reddit, TalkBass)
5. [ ] Contact music bloggers (send review copies)

#### Distribution
6. [ ] Set up payment processing (Stripe, Gumroad)
7. [ ] Create license key system (optional)
8. [ ] Set up download delivery
9. [ ] Create customer support email
10. [ ] Set up analytics (usage tracking, crash reporting)

#### Launch
11. [ ] Finalize pricing ($39.99)
12. [ ] Set early adopter discount ($29.99 for first 100)
13. [ ] Schedule launch date
14. [ ] Prepare launch announcement
15. [ ] Submit to software directories (AlternativeTo, Softpedia)

**Deliverables:**
- Landing page live
- Payment system operational
- Launch announcement ready
- Distribution channels set up

**Success Criteria:**
- All systems tested
- Launch date confirmed
- Marketing materials ready
- Support channels operational

---

## Post-Launch (Months 2-6)

### Roadmap Features

**Month 2:**
- [ ] Add polyphonic tuning (all 6 strings at once)
- [ ] Spectrum analyzer improvements
- [ ] Custom temperament support

**Month 3:**
- [ ] VST/AU plugin development (DAW integration)
- [ ] Session logging (pitch over time)
- [ ] Intonation tools (12th fret comparison)

**Month 4-6:**
- [ ] Windows installer with ASIO4ALL bundling
- [ ] Linux package manager submissions (apt, flatpak)
- [ ] Localization (Spanish, French, German, Japanese)

---

## Success Metrics

### Technical Metrics
- **Accuracy:** ±0.1 cent (verified with oscilloscope)
- **Latency:** <10ms end-to-end
- **CPU Usage:** <5% single core
- **Memory:** <50 MB resident
- **Startup Time:** <2 seconds

### Business Metrics (Year 1)
- **Conservative:** 500 sales ($19,995 revenue)
- **Target:** 2,000 sales ($79,980 revenue)
- **Optimistic:** 5,000 sales ($199,950 revenue)

### Quality Metrics
- **Beta Satisfaction:** >80%
- **User Reviews:** >4.5/5 average
- **Bug Reports:** <10 critical bugs per month
- **Support Load:** <2 hours/week

---

## Risk Mitigation

| Risk | Mitigation |
|------|------------|
| Submodule complexity | Comprehensive documentation, CI automation |
| Cross-platform audio issues | Extensive testing, fallback APIs |
| Achieving 0.1 cent accuracy | YIN algorithm proven, early validation testing |
| Market competition from Peterson | Price differentiation, Linux support, modern UI |
| GPL licensing issues | Use BSD/MIT alternatives (PFFFT, not FFTW) |

---

## Current Status

**Phase:** Phase 2 Complete ✅ / Phase 3 In Progress (71% Complete)
**Progress:** ~55% overall (Phases 0-2 complete, Phase 3 at 71%)
**Completed Recently:**
- ✅ Settings layer basic UI (toggle button, visibility control)
- ✅ Fixed span out-of-bounds crash in layer initialization
- ✅ Responsive window sizing and layout (GLFW framebuffer queries, size constraints)
- ✅ External GLSL shader files (geometry.vert/frag, text.vert/frag)
- ✅ TrueType font rendering with stb_truetype
- ✅ Configuration persistence (JSON save/load, platform-specific paths)
- ✅ Modern OpenGL rendering with color-coded tuning feedback
- ✅ Real-time pitch detection with YIN algorithm

**Next Priority Tasks (Phase 3):**
1. Device selection controls (populate settings UI with audio device dropdowns)
2. Multiple tuning modes (chromatic, standard, drop D, etc.)
3. Optional: Spectrum analyzer visualization

**Blockers:** None

**Last Updated:** 2025-11-23