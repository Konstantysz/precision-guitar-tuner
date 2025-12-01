# UI Polish & Onboarding - Implementation Plan

This document outlines the remaining UI polish tasks for v1.0.0 release.

## 1. Tooltips System

### Implementation

Add ImGui tooltips to all interactive controls.

**File**: `src/Layers/SettingsLayer.cpp`

```cpp
// Example tooltip implementation
if (ImGui::SliderFloat("Input Gain", &inputGain, 0.5f, 2.0f))
{
    // Update gain
}
if (ImGui::IsItemHovered())
{
    ImGui::SetTooltip("Boost or reduce input signal\nRange: 0.5x - 2.0x\nShortcut: Up/Down arrows");
}
```

### Controls Requiring Tooltips

#### Audio Settings

- **Input Device**: "Select your audio interface or USB cable"
- **Output Device**: "Choose speakers or headphones for audio feedback"
- **Sample Rate**: "48000 Hz recommended for best compatibility"
- **Buffer Size**: "Lower = less latency, higher = more stability\n128 frames = ~2.7ms, 256 frames = ~5.3ms"
- **Input Channel**: "Select which input your guitar is plugged into"
- **Input Gain**: "Boost weak signals or reduce clipping\nShortcut: Up/Down arrows"

#### Audio Feedback

- **Input Monitoring**: "Hear yourself play through speakers/headphones\nShortcut: Space"
- **Monitoring Volume**: "Output level for input monitoring"
- **Drone Mode**: "Continuous reference tone for ear training\nShortcut: D"
- **Polyphonic Mode**: "Play all six open strings simultaneously\nShortcut: P"
- **Reference Tone**: "Pure sine wave at detected frequency\nShortcut: R"
- **In-Tune Beep**: "Audio confirmation when perfectly in tune\nShortcut: B"

#### Tuning Settings

- **Tuning Mode**: "Chromatic detects any note\nPresets help verify correct string"
- **Reference Pitch**: "A4 frequency (440 Hz = concert pitch)"
- **Tolerance**: "How close to be 'in tune'\n±1.0 cent recommended"

## 2. Keyboard Shortcuts

### Already Documented (Need Implementation)

| Shortcut | Action | Priority |
|----------|--------|----------|
| `Space` | Toggle Input Monitoring | HIGH |
| `D` | Toggle Drone Mode | HIGH |
| `P` | Toggle Polyphonic Mode | MEDIUM |
| `R` | Toggle Reference Tone | MEDIUM |
| `B` | Toggle In-Tune Beep | MEDIUM |
| `M` | Mute All Audio Feedback | HIGH |
| `↑` / `↓` | Adjust Input Gain | HIGH |
| `Ctrl + ,` | Open Settings | HIGH |
| `Esc` | Close Settings | HIGH |
| `F11` | Toggle Fullscreen | LOW |

### Implementation Location

**File**: `src/PrecisionGuitarTuner.cpp` (main event loop)

```cpp
void PrecisionGuitarTuner::HandleKeyboardInput()
{
    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        audioLayer->ToggleInputMonitoring();
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_D))
    {
        audioLayer->ToggleDroneMode();
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_M))
    {
        audioLayer->MuteAllFeedback();
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        audioLayer->IncreaseInputGain(0.1f);
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        audioLayer->DecreaseInputGain(0.1f);
    }
    
    // Settings shortcuts
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Comma))
    {
        settingsLayer->Open();
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        settingsLayer->Close();
    }
}
```

## 3. First-Run Experience

### Welcome Dialog (Optional for v1.0.0)

Show on first launch:

1. Quick audio device selection
2. Buffer size recommendation based on system
3. Link to Quick Start Guide
4. "Don't show again" checkbox

**Implementation**: Add `firstRun` flag to `Config.h`

```cpp
struct AppConfig
{
    bool firstRun = true;
    bool showWelcomeDialog = true;
};
```

## 4. Help Menu / About Dialog

### Help Menu Items

- **Quick Start Guide** → Open `docs/QUICK_START.md` in browser
- **User Guide** → Open `docs/USER_GUIDE.md` in browser
- **Keyboard Shortcuts** → Show overlay with all shortcuts
- **About** → Version info, credits, license

### About Dialog Content

```
Precision Guitar Tuner v1.0.0

Professional-grade guitar tuner with ±0.1 cent accuracy

Built with:
- kappa-core (OpenGL UI framework)
- RtAudio (Cross-platform audio I/O)
- PFFFT (Fast Fourier Transform)
- YIN/MPM (Pitch detection algorithms)

License: MIT
GitHub: github.com/Konstantysz/precision-guitar-tuner
```

## 5. Visual Feedback Improvements

### Loading States

- Show "Initializing audio..." on startup
- Display "Searching for audio devices..." if none found
- "Listening..." indicator when ready

### Error Messages (User-Friendly)

Instead of: `"Failed to open audio stream"`
Show: `"Could not connect to audio device. Please check your connections and try again."`

### Status Bar

Add bottom status bar showing:

- Current input device
- Sample rate / Buffer size
- Latency estimate
- CPU usage (optional)

## 6. Accessibility

### Color Blind Support

- Ensure in-tune indicator uses both color AND position
- Add option for high-contrast mode
- Text labels for all visual indicators

### Screen Reader Support (Future)

- Label all controls with `aria-label` equivalents
- Announce pitch changes for blind users

## Implementation Priority for v1.0.0

### MUST HAVE (Blocking Release)

1. ✅ User Guide documentation
2. ✅ Quick Start Guide
3. ⏳ Keyboard shortcuts (Space, D, M, Up/Down, Ctrl+,, Esc)
4. ⏳ Tooltips for all settings controls

### SHOULD HAVE (High Value)

5. ⏳ Help menu with links to guides
6. ⏳ About dialog
7. ⏳ User-friendly error messages

### NICE TO HAVE (Can Defer to v1.1.0)

8. ⏳ First-run welcome dialog
9. ⏳ Status bar with device info
10. ⏳ Keyboard shortcuts overlay (F1)
11. ⏳ High-contrast mode

## Testing Checklist

- [ ] All tooltips display correctly
- [ ] All keyboard shortcuts work
- [ ] Settings can be opened/closed with Ctrl+, and Esc
- [ ] Audio feedback can be toggled with shortcuts
- [ ] Input gain responds to Up/Down arrows
- [ ] Help menu links open correct documentation
- [ ] About dialog shows correct version
- [ ] Error messages are user-friendly
- [ ] First-run experience works (if implemented)

## Notes

- Keep tooltips concise (1-2 lines max)
- Include keyboard shortcuts in tooltips where applicable
- Use consistent terminology across UI and documentation
- Test with keyboard-only navigation
- Ensure all interactive elements are accessible

---

**Target Completion**: Before v1.0.0 release
**Estimated Effort**: 8-12 hours
**Dependencies**: None (can be implemented incrementally)
