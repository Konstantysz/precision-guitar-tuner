/**
 * Precision Guitar Tuner
 * Professional-grade desktop tuner with ±0.1 cent accuracy
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include <Application.h>
#include <Logger.h>
#include <memory>

/**
 * Main application class for Precision Guitar Tuner
 * Extends Kappa::Application to provide the application lifecycle
 */
class PrecisionTunerApp : public Kappa::Application {
public:
    PrecisionTunerApp()
        : Application(Kappa::ApplicationSpecification{
            .name = "Precision Guitar Tuner",
            .windowSpecification = {
                .title = "Precision Guitar Tuner v1.0.0",
                .width = 1024,
                .height = 768
            }
        })
    {
        LOG_INFO("Precision Tuner initialized");

        // TODO: Push layers
        // PushLayer<AudioProcessingLayer>();
        // PushLayer<TunerVisualizationLayer>();
        // PushLayer<SettingsLayer>();
    }

    ~PrecisionTunerApp() override {
        LOG_INFO("Precision Tuner shutting down");
    }
};

/**
 * Application entry point
 * Creates and runs the kappa-core application
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    LOG_INFO("====================================");
    LOG_INFO("  Precision Guitar Tuner v1.0.0");
    LOG_INFO("====================================");

    // Create and run application
    auto app = std::make_unique<PrecisionTunerApp>();
    app->Run();

    return 0;
}
