/**
 * Precision Guitar Tuner
 * Professional-grade desktop tuner with ±0.1 cent accuracy
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include "AudioProcessingLayer.h"
#include "Config.h"
#include "TunerVisualizationLayer.h"
#include <Application.h>
#include <Logger.h>
#include <memory>

using namespace PrecisionTuner;
using namespace PrecisionTuner::Layers;

/**
 * Main application class for Precision Guitar Tuner
 * Extends Kappa::Application to provide the application lifecycle
 */
class PrecisionTunerApp : public Kappa::Application
{
public:
    PrecisionTunerApp()
        : Application(CreateApplicationSpec())
    {
        LOG_INFO("Precision Tuner initialized");

        // Load configuration
        config = Config::Load();
        LOG_INFO("Configuration loaded");

        // Push audio processing layer (manages audio I/O and pitch detection)
        PushLayer<AudioProcessingLayer>();

        // Push visualization layer (renders tuner display)
        // Note: TunerVisualizationLayer needs reference to AudioProcessingLayer
        // We'll use GetLayers() to access the audio layer
        auto layers = GetLayers();
        auto *audioLayer = dynamic_cast<AudioProcessingLayer *>(layers[0].get());
        if (audioLayer)
        {
            PushLayer<TunerVisualizationLayer>(*audioLayer);
        }

        LOG_INFO("All layers initialized");
    }

    ~PrecisionTunerApp() override
    {
        LOG_INFO("Precision Tuner shutting down");

        // Save configuration
        if (config.Save())
        {
            LOG_INFO("Configuration saved successfully");
        }
        else
        {
            LOG_ERROR("Failed to save configuration");
        }
    }

private:
    Config config;

    /**
     * Create application specification from loaded config
     * Called before constructor body, so uses default config
     */
    static Kappa::ApplicationSpecification CreateApplicationSpec()
    {
        // Load config to get window settings
        Config loadedConfig = Config::Load();

        return Kappa::ApplicationSpecification{
            .name = "Precision Guitar Tuner",
            .windowSpecification = { .title = "Precision Guitar Tuner v0.0.3-alpha",
                .width = static_cast<unsigned int>(loadedConfig.window.width),
                .height = static_cast<unsigned int>(loadedConfig.window.height),
                .isResizable = true  // Enable resizing for Phase 3 responsive layout
            }
        };
    }
};

/**
 * Application entry point
 * Creates and runs the kappa-core application
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    LOG_INFO("====================================");
    LOG_INFO("  Precision Guitar Tuner v0.0.3-alpha");
    LOG_INFO("  Audio Engine: YIN Pitch Detection");
    LOG_INFO("  Target Accuracy: ±0.1 cents");
    LOG_INFO("  Config System: ACTIVE");
    LOG_INFO("====================================");

    // Create and run application
    auto app = std::make_unique<PrecisionTunerApp>();
    app->Run();

    return 0;
}
