/**
 * Precision Guitar Tuner
 * Professional-grade desktop tuner with ±0.1 cent accuracy
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include "AudioProcessingLayer.h"
#include "TunerVisualizationLayer.h"
#include <Application.h>
#include <Logger.h>
#include <memory>

/**
 * Main application class for Precision Guitar Tuner
 * Extends Kappa::Application to provide the application lifecycle
 */
class PrecisionTunerApp : public Kappa::Application
{
public:
    PrecisionTunerApp()
        : Application(Kappa::ApplicationSpecification{ .name = "Precision Guitar Tuner",
              .windowSpecification = { .title = "Precision Guitar Tuner v0.0.1 - Audio Engine Active",
                  .width = 1024,
                  .height = 768 } })
    {
        LOG_INFO("Precision Tuner initialized");

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
    }
};

/**
 * Application entry point
 * Creates and runs the kappa-core application
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    LOG_INFO("====================================");
    LOG_INFO("  Precision Guitar Tuner v0.0.1");
    LOG_INFO("  Audio Engine: YIN Pitch Detection");
    LOG_INFO("  Target Accuracy: ±0.1 cents");
    LOG_INFO("====================================");

    // Create and run application
    auto app = std::make_unique<PrecisionTunerApp>();
    app->Run();

    return 0;
}
