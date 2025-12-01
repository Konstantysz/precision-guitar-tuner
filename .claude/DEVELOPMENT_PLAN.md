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
2. [x] Initialize git repository
3. [x] Create submodule repositories:
   - [x] lib-guitar-io
   - [x] lib-guitar-dsp
4. [x] Add kappa-core as submodule
5. [x] Create root CMakeLists.txt
6. [x] Set up vcpkg.json for dependencies
7. [x] Create DEPENDENCIES.md for version tracking
8. [x] Create .gitignore
9. [x] Create README.md

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

1. [x] Create AudioDevice class (RAII wrapper for RtAudio device info)
2. [x] Create AudioEngine class (manages RtAudio instance)
3. [x] Implement device enumeration
4. [ ] Implement device hot-plug detection
5. [ ] Create platform-specific audio stream initialization (ASIO/CoreAudio/ALSA)
6. [x] Implement audio callback with circular buffer
7. [ ] Add error handling and logging
8. [ ] Write unit tests for device management

#### Application Integration

1. [x] Create AudioProcessingLayer (kappa-core layer)
2. [x] Integrate lib-guitar-io into application
3. [x] Implement Rocksmith cable auto-detection
4. [x] Add device selection persistence (Config class)
5. [x] Test audio input on Windows (ASIO/WASAPI)

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

1. [x] Integrate PFFFT library (BSD license)
2. [x] Create FFT wrapper class
3. [x] Implement YIN pitch detection algorithm
4. [x] Add YIN confidence calculation
5. [ ] Implement MPM algorithm (fallback)
6. [x] Create NoteConverter utility (frequency ↔ note/cents)
7. [ ] Write comprehensive unit tests (test with generated sine waves)
8. [ ] Optimize for real-time performance (<5ms processing)

#### Application Integration

9. [x] Integrate lib-guitar-dsp into AudioProcessingLayer
10. [x] Implement pitch detection pipeline (buffer → YIN → frequency)
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

**Status:** 6 of 7 major features complete, 1 in progress (85%)
**Completed:** External GLSL shaders ✅, TrueType font rendering ✅, Configuration persistence ✅, Responsive layout ✅, Settings layer (basic UI) ✅, Multiple Tuning Modes ✅
**In Progress:** Device selection controls
**Deferred:** Spectrum analyzer (optional)

**Goals:**

- ✅ Externalize shaders to .glsl files (COMPLETE)
- ✅ Implement TrueType font rendering (COMPLETE)
- ✅ Configuration persistence (JSON save/load) (COMPLETE)
- ✅ Responsive window sizing and layout (COMPLETE)
- Settings layer with device selection
- ✅ Multiple tuning modes (chromatic, standard, drop D, etc.) (COMPLETE)
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

21. [x] Create SettingsLayer (kappa-core layer)
22. [x] Choose UI framework: ImGui (recommended) or custom OpenGL controls
23. [x] Add device selection dropdown (uses AudioProcessingLayer::GetAvailableDevices())
24. [x] Add reference pitch adjustment slider (A=430-450 Hz)
25. [x] Add tuning mode selector (chromatic, standard, drop D, etc.)
26. [ ] Runtime device switching (stop current, start new)
27. [x] Add SettingsLayer to application layer stack

#### Multiple Tuning Modes (NEW) ✅ COMPLETE

28. [x] Create TuningPresets.h/.cpp class
29. [x] Implement tuning preset system with note/frequency arrays
30. [x] Add tuning presets: Standard (EADGBE), Drop D, Drop C, DADGAD, Open G, etc.
31. [x] Add chromatic mode (current behavior - any note)
32. [x] Visual indicator for "closest target string" in standard tuning modes
33. [x] Tuning mode selector UI in SettingsLayer
34. [x] Persist selected tuning mode in Config

#### Optional: Spectrum Analyzer

35. [ ] FFT visualization using existing PFFFT from lib-guitar-dsp
36. [ ] Frequency magnitude spectrum bar graph
37. [ ] Harmonic peak highlighting
38. [ ] Toggle between tuner/spectrum modes

#### Application Polish ✅ COMPLETE

38. [x] Create Application class (kappa::Application subclass)
39. [x] Set up layer stack (Audio → Visualization)
40. [x] Implement window title and icon
41. [x] Add SettingsLayer to stack (pending implementation)

**Deliverables:**

- ✅ Functional tuner UI with modern OpenGL rendering
- ✅ External GLSL shader files
- ✅ TrueType font rendering for text display
- ✅ 60 FPS rendering with color-coded tuning feedback
- ✅ Configuration persistence (JSON save/load with platform paths)
- ✅ Responsive window layout with size constraints
- ✅ Settings layer with device selection
- ✅ Multiple tuning modes

**Success Criteria:**

- ✅ Needle responds in real-time (<50ms visual latency)
- ✅ UI is clear and readable
- ✅ No UI lag or stuttering at 60 FPS
- ✅ Settings persist between sessions (config save/load working)
- ✅ Window resize handled gracefully (400×300 min, 3840×2160 max)
- [x] Device switching works without restart

---

### Phase 4: Advanced Tuning Features (Week 6)

**Goals:**

- Multiple tuning modes (standard, drop, alternate)
- Chromatic mode
- Audio feedback (in-tune tone, reference pitch playback)
- Visual feedback improvements
- Accuracy refinement

**Tasks:**

#### Tuning System ✅ COMPLETE

1. [x] Create TuningPresets class
2. [x] Implement standard tuning (EADGBE)
3. [x] Implement drop tunings (Drop D, Drop C, Drop B)
4. [x] Implement alternate tunings (DADGAD, Open G, Open D)
5. [x] Add chromatic mode (any note detection)
6. [x] Create tuning preset selector UI
7. [x] Implement "closest string" indicator (for standard tuning)

#### Audio Feedback (NEW)

8. [x] Implement tone generator in lib-guitar-io (sine wave synthesis)
9. [x] Add "in-tune beep" feature (configurable on/off)
10. [x] Implement reference pitch playback (play target note)
11. [x] Add tone volume control in settings
12. [x] Create audio mixing (input + tone output)
13. [x] Add "drone mode" (continuous reference tone while tuning)
14. [x] Implement polyphonic tone generation (play full chord for checking tuning)

#### Visualization Enhancements

15. [ ] Add frequency spectrum display (optional)
16. [x] Improve needle smoothing (exponential moving average)
17. [x] Add color coding (red far, yellow close, green in-tune)
18. [ ] Implement cents history graph (optional)
19. [x] Add audio level meter (input gain indicator)

#### Accuracy Improvements

20. [x] Fine-tune YIN parameters for guitar frequencies
21. [x] Implement hybrid YIN/MPM approach (use MPM when YIN confidence < 0.8)
22. [x] Add harmonic rejection (ignore overtones)
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

### Phase 3.5: Visual Enhancement & Polish (Current Priority) 🎨 PARTIALLY COMPLETE

**Status:** Phase 1 (Textures & Asset Polish) Complete. Further polish deferred.
**Goal:** Transform from beta/development UI to premium professional-grade appearance

**Motivation:**

- Current retro gauge uses basic ImGui primitives
- Lacks realistic materials (chrome, aged paper, wood)
- No depth, shadows, or professional lighting
- Goal: Match or exceed $200 commercial tuner aesthetics

#### Phase 1: Textures & Asset Polish (Priority: High, Effort: 2-3 hours) ✅ COMPLETE

**Completed Visual Improvements:**

1. [x] **Wood Background** - Dark walnut grain texture with "Cover" scaling for professional appearance
2. [x] **Gauge Face** - Cream-colored aged paper texture with subtle grain and recessed shadow effect
3. [x] **Chrome Bezel** - Realistic metal texture with multi-layer programmatic shading (outer ring, inner ring, highlights)
4. [x] **Glass Overlay** - Programmatic rendering with circular highlights and bottom rim arc for crisp reflections
5. [x] **3D Needle** - Tapered design with programmatic soft shadow for depth and realism
6. [x] **Settings Gear Icon** - Custom vector-style gear icon in top-right corner with hover/active states
7. [x] **Brand Text** - "PRECISION SERIES" label with proper styling
8. [x] **Proper Layering** - Background → bezel → gauge face → needle → glass overlay

**Implementation Approach:**

- [x] Hybrid rendering: Textures for materials (wood, chrome, gauge face) + programmatic effects (glass, shadows, gear icon)
- [x] Texture loading via `stb_image.h` for PNG assets
- [x] ImGui primitives for UI elements and vector graphics
- [x] OpenGL texture rendering for background materials

**Deferred to Later Phases:**

- [ ] Custom vintage fonts for gauge labels
- [ ] Decorative screw details on bezel corners
- [ ] Custom brand emblem/logo

**Expected Impact:** 5x more professional appearance

#### Phase 2: Shader-Based Effects (Priority: Medium, Effort: 4-6 hours) ⚠️ DEFERRED

**Shader Development:**

1. [ ] Create `chrome_bezel.frag` - Realistic chrome with environment reflections
2. [ ] Create `gauge_face.frag` - Aged paper with radial lighting
3. [ ] Create `glass_overlay.frag` - Subtle window reflections and glare
4. [ ] Create `needle_shadow.frag` - Soft shadow rendering
5. [ ] Create `needle_motion.frag` - Motion blur for smooth needle movement

**Advanced Visual Effects:**
6. [ ] Implement ambient light rotation (subtle dynamic lighting)
7. [ ] Add depth of field (blur background, sharp foreground)
8. [ ] Spring-damped needle physics (realistic mechanical movement)
9. [ ] Fresnel reflections on chrome bezel
10. [ ] Radial shading on gauge face

**Hybrid Rendering Architecture:**

```cpp
void RenderRetroGauge() {
    // Layer 1: Background (shader or texture)
    RenderWoodBackground();
    
    // Layer 2: Gauge body with shadows (shader)
    RenderGaugeFaceAndBezel();
    
    // Layer 3: Tick marks and labels (ImGui - crisp text)
    RenderTickMarksAndLabels();
    
    // Layer 4: Needle with shadow and motion blur (shader)
    RenderNeedleWithPhysics();
    
    // Layer 5: Glass overlay with reflections (shader)
    RenderGlassOverlay();
    
    // Layer 6: Odometer display (ImGui text)
    RenderOdometerDisplay();
    
    // Layer 7: Effects (glow, sparkles if in-tune)
    if (isInTune) RenderInTuneEffects();
}
```

**Expected Impact:** Professional commercial-grade appearance

#### Phase 3: Advanced Polish & Effects (Priority: Low, Effort: Optional)

**Celebration Effects:**

1. [ ] Pulsing green glow when in-tune (animated)
2. [ ] Sparkle/shimmer effect on in-tune
3. [ ] Subtle particle effects

**Advanced Lighting:**
4. [ ] Normal mapping on bezel (raised/recessed depth)
5. [ ] Specular highlights on needle
6. [ ] Ambient occlusion in gauge recesses

**Audio-Visual Integration:**
7. [ ] Sound effects on needle movement (subtle click)
8. [ ] Haptic feedback (if supported)

**Performance Optimizations:**
9. [ ] Cache static elements (bezel, ticks, labels)
10. [ ] Only redraw needle on cents change
11. [ ] Use ImGui `ImDrawList::AddCallback` for GPU effects

**Expected Impact:** Premium $200+ app aesthetics

**Deliverables:**

- Phase 1: Textured retro gauge with professional appearance
- Phase 2: Shader-enhanced rendering with realistic materials
- Phase 3: Advanced effects and polish

**Success Criteria:**

- Looks indistinguishable from commercial $50-200 tuner apps
- Users comment on visual quality in beta feedback
- Smooth 60 FPS with all effects enabled
- Realistic materials (chrome, wood, aged paper)
- Professional color palette and typography

---

### Phase 5: Strobe Tuner (Week 7) ⚠️ REMOVED

*Note: Strobe tuner functionality has been removed to focus on a high-quality retro gauge experience.*

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
17. [x] Memory leak detection (Valgrind, ASAN)
18. [x] **Latency Optimization & Verification**
    - [x] Verify buffer size defaults (aim for 128/256 frames)
    - [x] Document latency impact in Config.h
    - [x] Optimize input callback for minimum audio pass-through latency

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

6. [ ] Set up download delivery
7. [ ] Create customer support email
8. [ ] Set up analytics (usage tracking, crash reporting)

#### Launch

9. [ ] Schedule launch date
10. [ ] Prepare launch announcement
11. [ ] Submit to software directories (AlternativeTo, Softpedia)

**Deliverables:**

- Landing page live
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
- **Audio Round-trip Latency:** <10ms (ASIO/CoreAudio)
- **Visual Latency:** <30ms (Input → Needle movement)
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

**Phase:** Phase 3.5 In Progress (Visual Enhancement)
**Progress:** ~80% overall
**Completed Recently:**

- ✅ **Visual Overhaul (Phase 1):**
  - Implemented high-quality textured retro gauge
  - Added "Cover" style wood background scaling
  - Created solid 3D chrome bezel with metallic shading
  - Implemented 3D ridged needle with pivot cap
  - Added realistic glass reflections (no artifacts)
  - Added recessed gauge face with inner shadows
- ✅ **Strobe Tuner Removed:** Simplified codebase to focus on premium retro gauge
- ✅ **Texture System:** Integrated `stb_image` and texture loading pipeline
- ✅ Fixed critical crash in MPM pitch detector (unsigned underflow)
- ✅ Fixed CI/CD build pipeline for Linux and macOS
- ✅ Audio Feedback (in-tune beep, reference pitch playback)
- ✅ Hybrid Pitch Detection (YIN/MPM) with harmonic rejection
- ✅ Settings layer basic UI (toggle button, visibility control)
- ✅ Fixed span out-of-bounds crash in layer initialization
- ✅ Responsive window sizing and layout (GLFW framebuffer queries, size constraints)
- ✅ External GLSL shader files (geometry.vert/frag, text.vert/frag)
- ✅ TrueType font rendering with stb_truetype
- ✅ Configuration persistence (JSON save/load, platform-specific paths)
- ✅ Modern OpenGL rendering with color-coded tuning feedback
- ✅ Real-time pitch detection with YIN algorithm
- ✅ Multiple Tuning Modes (7 presets + chromatic)
- ✅ Comprehensive Unit Tests for TuningPresets
- ✅ Refactored Tests to use Google Test
- ✅ Project Infrastructure (CMake, vcpkg, Git submodules)
- ✅ lib-guitar-io (Audio I/O library)
- ✅ Device selection controls (populate settings UI with audio device dropdowns)

**Next Priority Tasks (Phase 6):**

1. Cross-platform build and testing (macOS/Linux)
2. Further GUI polish (deferred)

**Blockers:** None

**Last Updated:** 2025-11-30
