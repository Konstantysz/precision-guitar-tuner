# Precision Guitar Tuner - User Guide

Welcome to Precision Guitar Tuner! This guide will help you get the most out of your professional-grade tuning experience.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Audio Setup](#audio-setup)
3. [Tuning Modes](#tuning-modes)
4. [Audio Feedback Features](#audio-feedback-features)
5. [Advanced Settings](#advanced-settings)
6. [Keyboard Shortcuts](#keyboard-shortcuts)
7. [Troubleshooting](#troubleshooting)

---

## Getting Started

### First Launch

When you first launch Precision Guitar Tuner, the application will:

1. Automatically detect your audio input device
2. Select the first available input channel
3. Set optimal buffer size for low latency (256 frames)
4. Configure a 48kHz sample rate

### Quick Tuning

1. **Plug in your guitar** to your audio interface or USB cable
2. **Play a string** - the tuner will automatically detect the pitch
3. **Watch the needle** - tune until it centers on 0 cents
4. **Green indicator** means you're in tune (within ±1 cent by default)

---

## Audio Setup

### Selecting Your Audio Device

**Settings → Audio → Input Device**

- **Recommended**: Use a dedicated audio interface (Focusrite, MOTU, RME)
- **Budget option**: Rocksmith Real Tone Cable
- **Avoid**: Built-in laptop microphones (high latency, poor quality)

### Optimizing for Low Latency

**Settings → Audio → Buffer Size**

| Buffer Size | Latency @ 48kHz | Use Case |
|-------------|-----------------|----------|
| 128 frames  | ~2.7ms         | Professional interfaces, powerful CPU |
| 256 frames  | ~5.3ms         | **Recommended default** |
| 512 frames  | ~10.7ms        | Older computers, stability issues |

**💡 Tip**: Start with 256 frames. Only reduce to 128 if you have a powerful CPU and professional interface.

### Input Channel Selection

If your interface has multiple inputs:

1. Go to **Settings → Audio → Input Channel**
2. Select the channel your guitar is plugged into (usually Channel 1)
3. Disable "Auto-select first available" for manual control

### Sample Rate

**Settings → Audio → Sample Rate**

- **48000 Hz** (Recommended) - Industry standard, best compatibility
- **44100 Hz** - CD quality, slightly higher CPU usage for conversion

---

## Tuning Modes

### Chromatic Mode (Default)

Detects any note across the entire musical range.

**Best for**:

- Standard tuning (E A D G B E)
- Drop tunings (Drop D, Drop C)
- Alternate tunings
- Bass guitar

### Preset Tuning Modes

**Settings → Tuning → Mode**

- **Standard** (E A D G B E)
- **Drop D** (D A D G B E)
- **Drop C** (C G C F A D)
- **DADGAD** (D A D G A D)
- **Open G** (D G D G B D)
- **Open D** (D A D F# A D)

**💡 Tip**: Preset modes help you quickly verify you're tuning to the correct string.

### Reference Pitch (A4)

**Settings → Tuning → Reference Pitch**

- **440 Hz** (Standard concert pitch)
- **441-444 Hz** (Baroque/period instruments)
- **432 Hz** (Alternative tuning)

---

## Audio Feedback Features

### Input Monitoring (Digital Amp)

**Settings → Audio Feedback → Enable Input Monitoring**

Hear yourself play through your computer speakers/headphones.

**Use cases**:

- Practice without an amp
- Silent practice with headphones
- Check your tone before recording

**Controls**:

- **Monitoring Volume**: Output level (0-100%)
- **Input Gain**: Boost/cut input signal (0.5x - 2.0x)

**⚠️ Warning**: Use headphones to avoid feedback loops!

### Drone Mode

**Settings → Audio Feedback → Enable Drone Mode**

Plays a continuous reference tone while you tune.

**Benefits**:

- Train your ear to recognize perfect pitch
- Tune by ear alongside visual feedback
- Useful for intonation practice

**Controls**:

- **Drone Volume**: Reference tone level
- **Reference Frequency**: Which note to drone (default: 440 Hz = A4)

### Polyphonic Mode

**Settings → Audio Feedback → Enable Polyphonic Mode**

Plays all six open string notes simultaneously.

**Best for**:

- Checking overall tuning stability
- Intonation reference
- Training relative pitch

### Reference Tone Generator

**Settings → Audio Feedback → Enable Reference Tone**

Plays a pure sine wave at the detected frequency.

**Use cases**:

- Verify tuner accuracy
- Ear training
- Pitch matching exercises

### In-Tune Beep

**Settings → Audio Feedback → Enable Beep**

Plays a confirmation beep when you're perfectly in tune.

**💡 Tip**: Useful for quick tuning without looking at the screen!

---

## Advanced Settings

### Tolerance (In-Tune Range)

**Settings → Tuning → Tolerance**

- **±0.5 cents**: Professional/studio work
- **±1.0 cents**: **Recommended** - Standard live performance
- **±2.0 cents**: Casual playing, beginners

### Output Device

**Settings → Audio → Output Device**

Separate from input device. Choose:

- **Studio monitors** for critical listening
- **Headphones** for silent practice
- **Built-in speakers** for casual use

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Space` | Toggle Input Monitoring |
| `D` | Toggle Drone Mode |
| `P` | Toggle Polyphonic Mode |
| `R` | Toggle Reference Tone |
| `B` | Toggle In-Tune Beep |
| `M` | Mute All Audio Feedback |
| `↑` / `↓` | Adjust Input Gain |
| `Ctrl + ,` | Open Settings |
| `Esc` | Close Settings |
| `F11` | Toggle Fullscreen |

**💡 Tip**: Hover over any control to see its keyboard shortcut!

---

## Troubleshooting

### "No audio input detected"

**Solutions**:

1. Check your guitar cable is plugged in
2. Verify audio interface is connected and powered on
3. Select correct input device in Settings → Audio
4. Increase Input Gain if signal is too weak
5. Check Windows/macOS audio permissions

### "Tuner is laggy or slow"

**Solutions**:

1. Increase buffer size to 512 frames
2. Close other audio applications
3. Update audio interface drivers
4. Disable audio feedback features temporarily

### "Pitch detection is unstable"

**Solutions**:

1. Play louder or increase Input Gain
2. Use neck pickup (warmer tone, easier to detect)
3. Avoid playing multiple strings at once
4. Check for electrical interference (move away from power supplies)
5. Fresh strings help! Old strings have poor harmonic content

### "Audio crackling or dropouts"

**Solutions**:

1. Increase buffer size to 512 frames
2. Close background applications
3. Use ASIO drivers on Windows (if available)
4. Update audio interface firmware
5. Disable Wi-Fi/Bluetooth temporarily

### "Can't hear input monitoring"

**Solutions**:

1. Check Output Device is correct
2. Increase Monitoring Volume
3. Verify output isn't muted in system settings
4. Use headphones to avoid feedback
5. Check audio interface output routing

---

## Tips for Best Performance

### Hardware Recommendations

**Excellent** (Professional):

- Focusrite Scarlett series
- MOTU M2/M4
- RME Babyface
- Universal Audio Apollo Twin

**Good** (Budget):

- Behringer U-Phoria series
- Rocksmith Real Tone Cable
- PreSonus AudioBox

**Avoid**:

- USB guitar cables (except Rocksmith)
- Built-in laptop microphones
- Bluetooth audio devices (too much latency)

### Tuning Best Practices

1. **Tune up, not down**: Always approach the target pitch from below
2. **Stretch new strings**: Tune, stretch, retune several times
3. **Check intonation**: Tune open string, then check 12th fret harmonic
4. **Temperature matters**: Guitars go sharp when warm, flat when cold
5. **Tune often**: Check tuning between every song in live situations

### Achieving <10ms Latency

1. Use ASIO drivers (Windows) or CoreAudio (macOS)
2. Set buffer size to 128 frames
3. Use a quality audio interface
4. Close unnecessary applications
5. Disable system sounds and notifications

---

## Support

**Found a bug?** Report it on [GitHub Issues](https://github.com/Konstantysz/precision-guitar-tuner/issues)

**Need help?** Check the [FAQ](https://github.com/Konstantysz/precision-guitar-tuner/wiki/FAQ)

**Feature request?** Open a [Discussion](https://github.com/Konstantysz/precision-guitar-tuner/discussions)

---

**Version**: 1.0.0  
**Last Updated**: 2025-12-01
