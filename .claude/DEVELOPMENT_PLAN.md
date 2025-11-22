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

### Phase 3: Basic UI and Visualization (Weeks 4-5)

**Goals:**
- Implement TunerVisualizationLayer
- Needle-style tuner display
- Frequency and cents offset display
- Basic settings panel

**Tasks:**

#### Rendering Layer
1. [ ] Create TunerVisualizationLayer (kappa-core layer)
2. [ ] Implement needle meter rendering (OpenGL)
3. [ ] Add note name display (current detected note)
4. [ ] Add frequency display (Hz)
5. [ ] Add cents offset display (±50 cents range)
6. [ ] Create "in-tune" indicator (green zone)
7. [ ] Design OpenGL shaders for smooth needle animation
8. [ ] Implement reference pitch line (target frequency)

#### Settings UI
9. [ ] Create SettingsLayer (kappa-core layer)
10. [ ] Add device selection dropdown
11. [ ] Add reference pitch adjustment (A=430-450 Hz)
12. [ ] Add tuning mode selector (chromatic, standard, drop D)
13. [ ] Implement settings persistence (save/load config)

#### Application Polish
14. [ ] Create Application class (kappa::Application subclass)
15. [ ] Set up layer stack (Audio → Visualization → Settings)
16. [ ] Implement window title and icon
17. [ ] Add FPS counter for debugging

**Deliverables:**
- Functional tuner UI (needle display)
- Device selection working
- Settings save/load working
- Responsive 60 FPS rendering

**Success Criteria:**
- Needle responds in real-time (<50ms visual latency)
- UI is clear and readable
- Settings persist between sessions
- No UI lag or stuttering

---

### Phase 4: Advanced Tuning Features (Week 6)

**Goals:**
- Multiple tuning modes (standard, drop, alternate)
- Chromatic mode
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

#### Visualization Enhancements
8. [ ] Add frequency spectrum display
9. [ ] Improve needle smoothing (exponential moving average)
10. [ ] Add color coding (red far, yellow close, green in-tune)
11. [ ] Implement cents history graph (optional)
12. [ ] Add audio level meter (input gain indicator)

#### Accuracy Improvements
13. [ ] Fine-tune YIN parameters for guitar frequencies
14. [ ] Implement hybrid YIN/MPM approach (use MPM when YIN confidence < 0.8)
15. [ ] Add harmonic rejection (ignore overtones)
16. [ ] Calibrate for different string gauges

**Deliverables:**
- 10+ tuning presets available
- Chromatic mode working
- Enhanced visual feedback
- ±0.1 cent accuracy achieved

**Success Criteria:**
- All tuning modes detect correctly
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

**Phase:** Phase 0 - Repository Setup
**Progress:** 10% (CLAUDE.md created)
**Next Task:** Initialize git repository and create submodule structure
**Blockers:** None

**Last Updated:** 2025-01-22