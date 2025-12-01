/**
 * Precision Guitar Tuner
 * Professional-grade desktop tuner with ±0.1 cent accuracy
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include <Logger.h>

#include "PrecisionGuitarTunerApp.h"

/**
 * Application entry point
 * Creates and runs the kappa-core application
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    // Set logger name before any logging
    Kappa::Logger::SetLoggerName("PrecisionGuitarTuner");

    LOG_INFO("====================================");
    LOG_INFO("  Precision Guitar Tuner v0.0.3-alpha");
    LOG_INFO("  Audio Engine: YIN Pitch Detection");
    LOG_INFO("  Target Accuracy: ±0.1 cents");
    LOG_INFO("  Config System: ACTIVE");
    LOG_INFO("====================================");

    // Create and run application
    auto app = std::make_unique<PrecisionGuitarTunerApp>();
    app->Run();

    return 0;
}
