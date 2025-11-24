# Dependency Versions

This file tracks all external dependencies and their pinned versions for reproducible builds.

## Git Submodules

| Component | Repository | Version/Commit | Date Pinned | Status |
|-----------|------------|----------------|-------------|---------|
| kappa-core | <https://github.com/Konstantysz/kappa-core> | v0.5.1 (66c2fc4) | 2025-11-22 | ✅ Active |
| lib-guitar-io | TBD (to be created) | TBD | TBD | 📝 Not Created |
| lib-guitar-dsp | <https://github.com/Konstantysz/lib-guitar-dsp> | v0.0.1 (65c93ae) | 2025-11-24 | ✅ Active |

## vcpkg Dependencies (kappa-core requirements)

| Package | Version | Purpose |
|---------|---------|---------|
| glfw3 | ≥3.3.8 | Window management and input handling |
| glad | ≥0.1.36 | OpenGL function loader |
| glm | ≥0.9.9 | OpenGL mathematics library |
| spdlog | ≥1.11.0 | Fast C++ logging library |

## Nested Submodule Dependencies (within lib-guitar-io and lib-guitar-dsp)

| Component | Repository | Version/Commit | Purpose | License |
|-----------|------------|----------------|---------|---------|
| RtAudio | <https://github.com/thestk/rtaudio> | TBD | Cross-platform audio I/O | MIT-like |
| PFFFT | <https://github.com/marton78/pffft> | TBD | Fast FFT library (GPL-free) | BSD |

## System Dependencies

### Windows

- **ASIO SDK** (optional): For ultra-low latency audio on Windows
  - Included via RtAudio build
  - Required for <10ms latency

- **WASAPI**: Built into Windows 10/11 (fallback audio API)

### macOS

- **CoreAudio**: Built into macOS 10.15+ (native audio API)
- **AudioToolbox**: Built into macOS (audio processing utilities)
- **CoreFoundation**: Built into macOS (system framework)

### Linux

- **ALSA**: Advanced Linux Sound Architecture
  - Ubuntu/Debian: `sudo apt install libasound2-dev`
  - Fedora: `sudo dnf install alsa-lib-devel`
  - Arch: `sudo pacman -S alsa-lib`

- **JACK** (optional): For professional audio routing
  - Ubuntu/Debian: `sudo apt install libjack-dev`
  - Fedora: `sudo dnf install jack-audio-connection-kit-devel`

## Build Tools

| Tool | Minimum Version | Purpose |
|------|-----------------|---------|
| CMake | 3.20+ | Build system generator |
| Git | 2.x | Version control and submodule management |
| C++ Compiler | C++20 support | MSVC 19.28+, GCC 10+, Clang 11+ |
| vcpkg | latest | C++ package manager |

## Updating Dependencies

### Updating Git Submodules

```bash
# Update all submodules to latest upstream
git submodule update --remote --merge

# Update specific submodule
cd external/kappa-core
git checkout <commit-hash-or-tag>
cd ../..
git add external/kappa-core
git commit -m "Update kappa-core to version X.Y.Z"

# IMPORTANT: Update this DEPENDENCIES.md file with new version info
```

### Updating vcpkg Dependencies

```bash
# Update vcpkg itself
cd <vcpkg-root>
git pull

# Update dependencies
vcpkg update
vcpkg upgrade --no-dry-run

# Update vcpkg.json if newer versions are required
```

## Version Pinning Policy

- **Submodules**: Always pin to specific commits (not branches)
- **vcpkg**: Specify minimum versions with `version>=`
- **System Libraries**: Document minimum supported versions
- **Testing**: Always test dependency updates in CI before merging

## License Compatibility

All dependencies must be compatible with commercial distribution:

✅ **Allowed Licenses**:

- MIT
- BSD (2-clause, 3-clause)
- Apache 2.0
- Zlib
- Public Domain

❌ **Prohibited Licenses**:

- GPL (any version) - viral copyleft
- LGPL (if statically linked)
- AGPL (any version)
- Commercial-only licenses

**Current Status**: ✅ All dependencies are GPL-free and commercially compatible

## Dependency Graph

```
precision-guitar-tuner
├── kappa-core (MIT)
│   ├── glfw3 (Zlib)
│   ├── glad (MIT/Public Domain)
│   ├── glm (MIT)
│   └── spdlog (MIT)
├── lib-guitar-io (MIT)
│   └── RtAudio (MIT-like)
└── lib-guitar-dsp (BSD)
    └── PFFFT (BSD)
```

## Notes

- This file should be updated whenever dependencies are added, removed, or updated
- Include the date when versions are pinned
- Test all updates in CI before merging to main
- Document any breaking changes from dependency updates

---

**Last Updated**: 2025-11-24
**Next Review**: Before each release
