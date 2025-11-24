/**
 * Configuration Management Implementation
 * Handles JSON serialization and platform-specific paths
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include "Config.h"
#include <Logger.h>
#include <cstdlib>
#include <fstream>

// Platform-specific includes (BEFORE namespace to avoid pollution)
#ifdef _WIN32
#include <Windows.h>
#include <shlobj.h>
#elif defined(__APPLE__)
#include <pwd.h>
#include <unistd.h>
#elif defined(__linux__)
#include <pwd.h>
#include <unistd.h>
#endif

namespace PrecisionTuner
{

    std::filesystem::path Config::GetDefaultConfigPath()
    {
        std::filesystem::path configDir;

#ifdef _WIN32
        // Windows: %APPDATA%/PrecisionTuner
        char *appData = nullptr;
        size_t len = 0;
        if (_dupenv_s(&appData, &len, "APPDATA") == 0 && appData != nullptr)
        {
            configDir = std::filesystem::path(appData) / "PrecisionTuner";
            free(appData);
        }
        else
        {
            // Fallback to current directory
            configDir = std::filesystem::current_path();
        }
#elif defined(__APPLE__)
        // macOS: ~/Library/Application Support/PrecisionTuner
        const char *home = getenv("HOME");
        if (home == nullptr)
        {
            struct passwd *pw = getpwuid(getuid());
            home = pw->pw_dir;
        }
        configDir = std::filesystem::path(home) / "Library" / "Application Support" / "PrecisionTuner";
#elif defined(__linux__)
        // Linux: ~/.config/PrecisionTuner
        const char *xdgConfig = getenv("XDG_CONFIG_HOME");
        if (xdgConfig != nullptr && xdgConfig[0] != '\0')
        {
            configDir = std::filesystem::path(xdgConfig) / "PrecisionTuner";
        }
        else
        {
            const char *home = getenv("HOME");
            if (home == nullptr)
            {
                struct passwd *pw = getpwuid(getuid());
                if (pw != nullptr)
                {
                    home = pw->pw_dir;
                }
            }
            if (home != nullptr)
            {
                configDir = std::filesystem::path(home) / ".config" / "PrecisionTuner";
            }
            else
            {
                configDir = std::filesystem::current_path();
            }
        }
#else
        // Fallback: current directory
        configDir = std::filesystem::current_path();
#endif

        return configDir / "config.json";
    }

    Config Config::Load(const std::filesystem::path &path)
    {
        try
        {
            // Check if file exists
            if (!std::filesystem::exists(path))
            {
                LOG_INFO("Config file not found at: {}. Using defaults.", path.string());
                return GetDefault();
            }

            // Read file
            std::ifstream file(path);
            if (!file.is_open())
            {
                LOG_ERROR("Failed to open config file: {}", path.string());
                return GetDefault();
            }

            // Parse JSON
            nlohmann::json jsonData;
            file >> jsonData;
            file.close();

            // Deserialize
            Config config = jsonData.get<Config>();

            // Version check (future-proofing for config migrations)
            if (config.version != 1)
            {
                LOG_WARN("Config version mismatch. Expected 1, got {}. Using defaults.", config.version);
                return GetDefault();
            }

            LOG_INFO("Configuration loaded from: {}", path.string());
            LOG_DEBUG("  Audio Device: {} (ID: {})", config.audio.deviceName, config.audio.deviceId);
            LOG_DEBUG("  Reference Pitch: {} Hz", config.tuning.referencePitch);
            LOG_DEBUG("  Window Size: {}x{}", config.window.width, config.window.height);

            return config;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Failed to load config: {}. Using defaults.", e.what());
            return GetDefault();
        }
    }

    bool Config::Save(const std::filesystem::path &path) const
    {
        try
        {
            // Create parent directories if they don't exist
            std::filesystem::path parentDir = path.parent_path();
            if (!parentDir.empty() && !std::filesystem::exists(parentDir))
            {
                std::filesystem::create_directories(parentDir);
                LOG_INFO("Created config directory: {}", parentDir.string());
            }

            // Serialize to JSON
            nlohmann::json jsonData = *this;

            // Write to file with pretty formatting
            std::ofstream file(path);
            if (!file.is_open())
            {
                LOG_ERROR("Failed to open config file for writing: {}", path.string());
                return false;
            }

            file << jsonData.dump(4); // 4-space indentation
            file.close();

            LOG_INFO("Configuration saved to: {}", path.string());
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Failed to save config: {}", e.what());
            return false;
        }
    }

    Config Config::GetDefault()
    {
        Config config;

        // Window defaults
        config.window.width = 1024;
        config.window.height = 768;
        config.window.posX = -1; // Centered
        config.window.posY = -1; // Centered
        config.window.isMaximized = false;

        // Audio defaults
        config.audio.deviceId = -1; // Auto-select default device
        config.audio.deviceName = "Default Audio Device";
        config.audio.sampleRate = 48000;
        config.audio.bufferSize = 256;

        // Tuning defaults
        config.tuning.mode = TuningMode::Chromatic;
        config.tuning.referencePitch = 440.0f; // Standard A440
        config.tuning.tolerance = 1.0f;        // ±1 cent in-tune range

        // Config version
        config.version = 1;

        return config;
    }

} // namespace PrecisionTuner
