#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <cstring>

namespace PrecisionTuner {

/**
 * Tuning mode enumeration
 */
enum class TuningMode {
    Chromatic,
    Standard,
    DropD,
    DropC,
    DADGAD,
    OpenG,
    OpenD
};

/**
 * Window configuration
 */
struct WindowConfig {
    int width = 1024;
    int height = 768;
    int posX = -1; // -1 means centered
    int posY = -1; // -1 means centered
    bool isMaximized = false;
    static constexpr int MIN_WIDTH = 400;
    static constexpr int MIN_HEIGHT = 300;
    static constexpr int MAX_WIDTH = 3840; // 4K resolution
    static constexpr int MAX_HEIGHT = 2160; // 4K resolution
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WindowConfig, width, height, posX, posY, isMaximized)
};

/**
 * Audio device configuration with feedback settings
 */
struct AudioConfig {
    int deviceId = -1;               // Input device ID (-1 means default)
    std::string deviceName;          // Input device name for display/matching
    int sampleRate = 48000;          // Sample rate in Hz
    int bufferSize = 256;            // Buffer size in frames
    int inputChannel = 0;            // Input channel index (0‑based)
    bool autoSelectInput = true;     // Automatically select first available input channel
    
    // Output device configuration
    int outputDeviceId = -1;         // Output device ID (-1 means default)
    std::string outputDeviceName;    // Output device name for display/matching
    
    // Feedback options
    bool enableBeep = false;         // Enable in‑tune beep feedback
    float beepVolume = 0.5f;         // Volume for beep (0.0‑1.0)
    bool enableReference = false;    // Enable reference pitch playback
    float referenceVolume = 0.5f;    // Volume for reference tone
    float referenceFrequency = 440.0f; // Frequency for reference tone (Hz)
    bool enableInputMonitoring = false; // Enable input monitoring (digital amp)
    float monitoringVolume = 0.5f;   // Volume for monitoring output
    float inputGain = 1.0f;          // Gain for input signal (1.0 = no change)
    
    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AudioConfig, deviceId, deviceName, sampleRate, bufferSize, inputChannel, autoSelectInput, outputDeviceId, outputDeviceName, enableBeep, beepVolume, enableReference, referenceVolume, referenceFrequency, enableInputMonitoring, monitoringVolume, inputGain)
};

/**
 * Tuning configuration
 */
struct TuningConfig {
    TuningMode mode = TuningMode::Chromatic;
    float referencePitch = 440.0f; // A4 reference frequency (Hz)
    float tolerance = 1.0f;       // In‑tune tolerance in cents
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TuningConfig, mode, referencePitch, tolerance)
};

/**
 * Main configuration structure
 */
struct Config {
    WindowConfig window;
    AudioConfig audio;
    TuningConfig tuning;
    int version = 1; // Config file format version
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, window, audio, tuning, version)
    static std::filesystem::path GetDefaultConfigPath();
    static Config Load(const std::filesystem::path &path = GetDefaultConfigPath());
    bool Save(const std::filesystem::path &path = GetDefaultConfigPath()) const;
    static Config GetDefault();
};

} // namespace PrecisionTuner
