# Precision Guitar Tuner - Product Roadmap

This document outlines the post-launch feature roadmap for Precision Guitar Tuner beyond v1.0.0. For current development status and v1.0.0 release tasks, see [DEVELOPMENT_PLAN.md](../.claude/DEVELOPMENT_PLAN.md).

**Last Updated:** 2025-12-02
**Current Version:** v1.0.0 (in development)

---

## Vision Statement

Position Precision Guitar Tuner as the premier cross-platform guitar tuning solution, offering professional-grade accuracy and features at an accessible price point. Expand from basic tuning to comprehensive guitar maintenance and intonation tools.

---

## Post-Launch Roadmap

### v1.1.0 (Month 2)

**Theme:** Enhanced Detection & Visualization

#### Polyphonic Tuning
- Detect all 6 strings simultaneously using spectral analysis
- Visual indicator for each string's tuning status
- Faster workflow for experienced players
- Requires advanced DSP (harmonic separation)

#### Spectrum Analyzer Improvements
- Real-time frequency magnitude display
- Harmonic peak highlighting
- Adjustable frequency range and resolution
- Toggle between tuner/spectrum modes
- Educational tool for understanding guitar harmonics

#### Custom Temperament Support
- Equal temperament (default)
- Just intonation
- Pythagorean tuning
- Historical temperaments (Meantone, Well-tempered)
- Custom user-defined temperaments
- Educational resource for music theory

**Estimated Effort:** 4-6 weeks
**Risk:** Polyphonic detection complexity may require significant R&D

---

### v1.2.0 (Month 3)

**Theme:** DAW Integration & Session Tools

#### VST/AU Plugin Development
- DAW integration (Reaper, Ableton, Logic, etc.)
- Inline tuning during recording sessions
- Sidechain input support
- Cross-platform plugin architecture (JUCE framework)
- Separate product or bundled with desktop app

#### Session Logging
- Pitch tracking over time
- Tuning drift analysis
- Temperature/humidity correlation (manual input)
- Export to CSV for analysis
- Visual graphs of tuning stability
- Useful for luthiers and professional players

#### Intonation Tools
- 12th fret comparison test
- Automated saddle adjustment recommendations
- String action analysis
- Nut slot height suggestions
- Comprehensive guitar setup assistant

**Estimated Effort:** 6-8 weeks
**Risk:** VST/AU certification and testing across DAWs

---

### v1.3.0 (Months 4-6)

**Theme:** Distribution & Accessibility

#### Windows Installer with ASIO4ALL
- MSI installer package
- Optional ASIO4ALL bundling (with permission)
- Automatic driver installation
- Simplifies setup for non-technical users
- Reduces support burden

#### Linux Package Manager Submissions
- Submit to apt repositories (Debian/Ubuntu)
- Flatpak package for universal Linux support
- AUR package for Arch Linux
- AppImage for portable Linux distribution
- Expand Linux user base

#### Localization (i18n)
- Spanish translation (large guitar market)
- French translation (Europe)
- German translation (Europe)
- Japanese translation (Asia)
- Crowdsourced translations (community contributions)
- Expand international market reach

**Estimated Effort:** 4-6 weeks (largely administrative)
**Risk:** Package manager approval processes can be slow

---

## Feature Backlog (Future Consideration)

### Advanced Features

- **Strobe Tuner Mode** (deferred from Phase 5)
  - High-precision visual feedback
  - Sub-cent accuracy visualization
  - Professional studio feature

- **MIDI Output**
  - Convert guitar input to MIDI notes
  - Drive virtual instruments
  - Educational tool for note recognition

- **Automatic Tuning Assistant**
  - Computer vision to detect which tuning peg to turn
  - Augmented reality overlay (mobile version)
  - Step-by-step tuning guidance for beginners

- **Mobile Versions**
  - iOS app (App Store)
  - Android app (Google Play)
  - Shared codebase with desktop (consider Flutter/React Native)

### Enterprise Features

- **Multi-User License Management**
  - School/institution licenses
  - Floating licenses for music studios
  - Volume discounts

- **Cloud Sync**
  - Settings sync across devices
  - Session history backup
  - Tuning profiles (per-guitar configurations)

---

## Success Metrics

### v1.0.0 Release Goals (Year 1)

**Technical:**
- ✅ Accuracy: ±0.1 cent (competitive with Peterson StroboSoft)
- ✅ Latency: <10ms end-to-end (input → detection → display)
- ✅ Update Rate: 60 Hz UI refresh
- ✅ CPU Usage: <5% on modern hardware (single core)
- ✅ Memory: <50 MB resident (no leaks in long-running sessions)

**Business:**
- **Conservative:** 500 sales ($19,995 revenue at $39.99/unit)
- **Target:** 2,000 sales ($79,980 revenue)
- **Optimistic:** 5,000 sales ($199,950 revenue)

**Quality:**
- **Beta Satisfaction:** >80%
- **User Reviews:** >4.5/5 average
- **Bug Reports:** <10 critical bugs per month
- **Support Load:** <2 hours/week

### Post-Launch Metrics (Year 2)

**Growth:**
- **User Base:** 10,000+ active users
- **Monthly Revenue:** $5,000+ (mix of new sales + plugin sales)
- **Market Position:** Top 3 cross-platform guitar tuners

**Engagement:**
- **Daily Active Users:** >30% of user base
- **Session Duration:** >5 minutes average
- **Feature Adoption:** >50% use advanced features (drone, polyphonic, etc.)

**Distribution:**
- **Platform Mix:** 60% Windows, 30% macOS, 10% Linux
- **Referral Rate:** >15% (word-of-mouth growth)

---

## Risk Mitigation

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Submodule complexity | Medium | Low | Comprehensive documentation, CI automation |
| Cross-platform audio issues | High | Medium | Extensive testing, fallback APIs |
| Market competition from Peterson | High | High | Price differentiation, Linux support, modern UI |
| GPL licensing issues | High | Low | Use BSD/MIT alternatives (PFFFT, not FFTW) |
| Polyphonic detection accuracy | Medium | Medium | R&D phase before commitment, fallback to enhanced monophonic |
| VST/AU plugin compatibility | High | Medium | Test across major DAWs, JUCE framework (proven) |
| Localization quality | Low | Medium | Native speaker reviews, community validation |
| Distribution platform approval | Low | High | Follow guidelines strictly, allow extra time |

---

## Open Questions

### Business Model

- **Pricing Strategy:** One-time purchase vs. subscription?
  - Current plan: $39.99 one-time for desktop app
  - VST/AU plugin: Separate purchase ($29.99) or bundled?
  - Mobile apps: Free with ads vs. paid ($4.99)?

- **Update Policy:** Free updates forever vs. paid major upgrades?
  - Current plan: Free updates for v1.x
  - v2.0 (if needed) would be paid upgrade with discount for existing users

### Technical Decisions

- **Plugin Framework:** JUCE (proven, large) vs. iPlug2 (lighter, newer)?
  - Leaning toward JUCE for ecosystem maturity

- **Mobile Strategy:** Native (Swift/Kotlin) vs. Cross-platform (Flutter)?
  - Consider after desktop app establishes market fit

- **Cloud Infrastructure:** Self-hosted vs. Firebase vs. AWS?
  - Only if cloud sync becomes a priority feature

---

## Community & Ecosystem

### Open Source Strategy

- **Core Application:** Proprietary (commercial product)
- **Libraries:**
  - lib-guitar-io: **Open source (MIT)** - benefits ecosystem
  - lib-guitar-dsp: **Open source (BSD)** - benefits ecosystem
  - kappa-core: Already open source (MIT)

- **Benefits:** Community contributions, educational value, goodwill
- **Risks:** Competition from forks (mitigated by superior UX and support)

### Developer Engagement

- **GitHub Discussions:** Feature requests, bug reports
- **Documentation:** Comprehensive API docs for library contributors
- **Example Projects:** Tutorials using lib-guitar-io/lib-guitar-dsp
- **Blog:** Development insights, DSP explanations

---

## Timeline Summary

| Version | Release | Theme | Key Features |
|---------|---------|-------|--------------|
| v1.0.0 | Month 1 | Launch | Core tuning, multi-platform, professional UI |
| v1.1.0 | Month 2 | Enhanced Detection | Polyphonic, spectrum analyzer, temperaments |
| v1.2.0 | Month 3 | DAW Integration | VST/AU plugin, session logging, intonation tools |
| v1.3.0 | Months 4-6 | Distribution | Installers, package managers, localization |
| v2.0.0 | Year 2 | TBD | Mobile apps, cloud sync, enterprise features |

---

**For current development status, see [DEVELOPMENT_PLAN.md](../.claude/DEVELOPMENT_PLAN.md)**
