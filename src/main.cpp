/**
 * Precision Guitar Tuner
 * Professional-grade desktop tuner with ±0.1 cent accuracy
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include <kappa/Application.hpp>
#include <kappa/Log.hpp>

/**
 * Main application class for Precision Guitar Tuner
 * Extends kappa::Application to provide the application lifecycle
 */
class PrecisionTunerApp : public kappa::Application {
public:
    PrecisionTunerApp()
        : Application({
            .title = "Precision Guitar Tuner v1.0.0",
            .width = 1024,
            .height = 768,
            .vsync = true
        })
    {
        KAPPA_INFO("Precision Tuner initialized");
    }

    ~PrecisionTunerApp() override {
        KAPPA_INFO("Precision Tuner shutting down");
    }

protected:
    void onInit() override {
        KAPPA_INFO("Application initialized");

        // TODO: Push layers
        // pushLayer(new AudioProcessingLayer());
        // pushLayer(new TunerVisualizationLayer());
        // pushLayer(new SettingsLayer());
    }

    void onUpdate([[maybe_unused]] float deltaTime) override {
        // Application-level update logic (if needed)
    }

    void onRender() override {
        // Application-level rendering (if needed)
    }
};

/**
 * Application entry point
 * Creates and runs the kappa-core application
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    // Initialize kappa logging
    kappa::Log::init();

    KAPPA_INFO("====================================");
    KAPPA_INFO("  Precision Guitar Tuner v1.0.0");
    KAPPA_INFO("====================================");

    // Create and run application
    auto app = std::make_unique<PrecisionTunerApp>();
    app->run();

    return 0;
}
