/**
 * Configuration Management for Precision Guitar Tuner
 * Handles persistent application settings with JSON serialization
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace PrecisionTuner
{

/**
 * Tuning mode enumeration
 * Defines the available tuning modes for the application
 */
enum class TuningMode
{
    Chromatic,  // Detect any note (current behavior)
    Standard,   // Standard guitar tuning (EADGBE)
    DropD,      // Drop D tuning (DADGBE)
    DropC,      // Drop C tuning (CGCFAD)
    DADGAD,     // DADGAD alternate tuning
    OpenG,      // Open G tuning (DGDGBD)
    OpenD       // Open D tuning (DADF#AD)
};

/**
 * Window state configuration
 * Stores window position and size for persistence
 */
struct WindowConfig
{
    int width = 1024;
    int height = 768;
    int posX = -1;  // -1 means centered
    int posY = -1;  // -1 means centered
    bool isMaximized = false;

    // Window constraints
    static constexpr int MIN_WIDTH = 400;
    static constexpr int MIN_HEIGHT = 300;
    static constexpr int MAX_WIDTH = 3840;  // 4K resolution
    static constexpr int MAX_HEIGHT = 2160; // 4K resolution

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WindowConfig, width, height, posX, posY, isMaximized)
};

/**
 * Audio device configuration
 * Stores selected audio device settings
 */
struct AudioConfig
{
    int deviceId = -1;        // -1 means default device
    std::string deviceName;   // Device name for display/matching
    int sampleRate = 48000;   // Sample rate in Hz
    int bufferSize = 256;     // Buffer size in frames

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AudioConfig, deviceId, deviceName, sampleRate, bufferSize)
};

/**
 * Tuning configuration
 * Stores tuning-related settings
 */
struct TuningConfig
{
    TuningMode mode = TuningMode::Chromatic;
    float referencePitch = 440.0f;  // A4 reference frequency (Hz)
    float tolerance = 1.0f;          // In-tune tolerance in cents

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TuningConfig, mode, referencePitch, tolerance)
};

/**
 * Main configuration structure
 * Contains all application settings
 */
struct Config
{
    WindowConfig window;
    AudioConfig audio;
    TuningConfig tuning;
    int version = 1;  // Config file format version

    // JSON serialization
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, window, audio, tuning, version)

    /**
     * Get the default configuration file path
     * Platform-specific user config directory
     *
     * Windows: %APPDATA%/PrecisionTuner/config.json
     * macOS: ~/Library/Application Support/PrecisionTuner/config.json
     * Linux: ~/.config/PrecisionTuner/config.json
     */
    static std::filesystem::path GetDefaultConfigPath();

    /**
     * Load configuration from file
     * Returns default config if file doesn't exist or is invalid
     *
     * @param path Path to config file (uses default if not specified)
     * @return Loaded configuration or default
     */
    static Config Load(const std::filesystem::path &path = GetDefaultConfigPath());

    /**
     * Save configuration to file
     * Creates parent directories if needed
     *
     * @param path Path to save config file (uses default if not specified)
     * @return true if save successful, false otherwise
     */
    bool Save(const std::filesystem::path &path = GetDefaultConfigPath()) const;

    /**
     * Get default configuration
     * @return Default configuration with safe defaults
     */
    static Config GetDefault();
};

}  // namespace PrecisionTuner
