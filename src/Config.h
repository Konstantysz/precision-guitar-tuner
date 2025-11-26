#pragma once

#include <nlohmann/json.hpp>
#include <cstring>
#include <filesystem>
#include <optional>

namespace PrecisionTuner
{

    /**
     * Tuning mode enumeration
     */
    enum class TuningMode
    {
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
    struct WindowConfig
    {
        int width = 1024;                       ///< Window width in pixels
        int height = 768;                       ///< Window height in pixels
        int posX = -1;                          ///< Window X position (-1 means centered)
        int posY = -1;                          ///< Window Y position (-1 means centered)
        bool isMaximized = false;               ///< Whether the window is maximized
        static constexpr int MIN_WIDTH = 400;   ///< Minimum allowed width
        static constexpr int MIN_HEIGHT = 300;  ///< Minimum allowed height
        static constexpr int MAX_WIDTH = 3840;  ///< Maximum allowed width (4K)
        static constexpr int MAX_HEIGHT = 2160; ///< Maximum allowed height (4K)

        // JSON serialization
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(WindowConfig, width, height, posX, posY, isMaximized)
    };

    /**
     * Audio device configuration with feedback settings
     */
    struct AudioConfig
    {
        int deviceId = -1;           ///< Input device ID (-1 means default)
        std::string deviceName;      ///< Input device name for display/matching
        int sampleRate = 48000;      ///< Sample rate in Hz
        int bufferSize = 256;        ///< Buffer size in frames
        int inputChannel = 0;        ///< Input channel index (0-based)
        bool autoSelectInput = true; ///< Automatically select first available input channel

        // Output device configuration
        int outputDeviceId = -1;      ///< Output device ID (-1 means default)
        std::string outputDeviceName; ///< Output device name for display/matching

        // Feedback options
        bool enableBeep = false;            ///< Enable in-tune beep feedback
        float beepVolume = 0.5f;            ///< Volume for beep (0.0-1.0)
        bool enableReference = false;       ///< Enable reference pitch playback
        float referenceVolume = 0.5f;       ///< Volume for reference tone
        float referenceFrequency = 440.0f;  ///< Frequency for reference tone (Hz)
        bool enableInputMonitoring = false; ///< Enable input monitoring (digital amp)
        float monitoringVolume = 0.5f;      ///< Volume for monitoring output
        float inputGain = 1.0f;             ///< Gain for input signal (1.0 = no change)

        // Advanced feedback modes
        bool enableDroneMode = false;      ///< Enable continuous reference tone (drone)
        bool enablePolyphonicMode = false; ///< Enable polyphonic chord playback
    };

    // Custom JSON serialization for AudioConfig to handle missing keys gracefully
    inline void to_json(nlohmann::json &j, const AudioConfig &config)
    {
        j = nlohmann::json{ { "deviceId", config.deviceId },
            { "deviceName", config.deviceName },
            { "outputDeviceId", config.outputDeviceId },
            { "outputDeviceName", config.outputDeviceName },
            { "enableBeep", config.enableBeep },
            { "beepVolume", config.beepVolume },
            { "enableReference", config.enableReference },
            { "referenceVolume", config.referenceVolume },
            { "referenceFrequency", config.referenceFrequency },
            { "enableInputMonitoring", config.enableInputMonitoring },
            { "monitoringVolume", config.monitoringVolume },
            { "inputGain", config.inputGain },
            { "enableDroneMode", config.enableDroneMode },
            { "enablePolyphonicMode", config.enablePolyphonicMode } };
    }

    inline void from_json(const nlohmann::json &j, AudioConfig &config)
    {
        config.deviceId = j.value("deviceId", AudioConfig{}.deviceId);
        config.deviceName = j.value("deviceName", AudioConfig{}.deviceName);
        config.outputDeviceId = j.value("outputDeviceId", AudioConfig{}.outputDeviceId);
        config.outputDeviceName = j.value("outputDeviceName", AudioConfig{}.outputDeviceName);
        config.enableBeep = j.value("enableBeep", AudioConfig{}.enableBeep);
        config.beepVolume = j.value("beepVolume", AudioConfig{}.beepVolume);
        config.enableReference = j.value("enableReference", AudioConfig{}.enableReference);
        config.referenceVolume = j.value("referenceVolume", AudioConfig{}.referenceVolume);
        config.referenceFrequency = j.value("referenceFrequency", AudioConfig{}.referenceFrequency);
        config.enableInputMonitoring = j.value("enableInputMonitoring", AudioConfig{}.enableInputMonitoring);
        config.monitoringVolume = j.value("monitoringVolume", AudioConfig{}.monitoringVolume);
        config.inputGain = j.value("inputGain", AudioConfig{}.inputGain);
        config.enableDroneMode = j.value("enableDroneMode", AudioConfig{}.enableDroneMode);
        config.enablePolyphonicMode = j.value("enablePolyphonicMode", AudioConfig{}.enablePolyphonicMode);
    }

    /**
     * Tuning configuration
     */
    struct TuningConfig
    {
        TuningMode mode = TuningMode::Chromatic; ///< Active tuning mode
        float referencePitch = 440.0f;           ///< A4 reference frequency (Hz)
        float tolerance = 1.0f;                  ///< In-tune tolerance in cents

        // JSON serialization
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(TuningConfig, mode, referencePitch, tolerance)
    };

    /**
     * Main configuration structure
     */
    struct Config
    {
        WindowConfig window; ///< Window settings
        AudioConfig audio;   ///< Audio settings
        TuningConfig tuning; ///< Tuning settings
        int version = 1;     ///< Config file format version

        // JSON serialization
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, window, audio, tuning, version)

        /**
         * @brief Get the default configuration file path
         * @return Path to the config.json file
         */
        static std::filesystem::path GetDefaultConfigPath();

        /**
         * @brief Load configuration from file
         * @param path Path to the config file (defaults to GetDefaultConfigPath())
         * @return Loaded configuration or default if load fails
         */
        static Config Load(const std::filesystem::path &path = GetDefaultConfigPath());

        /**
         * @brief Save configuration to file
         * @param path Path to the config file (defaults to GetDefaultConfigPath())
         * @return true if save successful, false otherwise
         */
        bool Save(const std::filesystem::path &path = GetDefaultConfigPath()) const;

        /**
         * @brief Get default configuration
         * @return Default configuration instance
         */
        static Config GetDefault();
    };

} // namespace PrecisionTuner
