#pragma once

#include "AudioProcessingLayer.h"
#include "Config.h"
#include "SettingsLayer.h"
#include "TunerVisualizationLayer.h"
#include <Application.h>
#include <Logger.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <memory>

/**
 * Main application class for Precision Guitar Tuner
 * Extends Kappa::Application to provide the application lifecycle
 */
class PrecisionGuitarTunerApp : public Kappa::Application
{
public:
    /**
     * @brief Constructs the application
     * Initializes window limits, loads config, and sets up layers.
     */
    PrecisionGuitarTunerApp();

    ~PrecisionGuitarTunerApp() override;

protected:
    void BeginFrame() override;

    void EndFrame() override;

private:
    /**
     * Initialize ImGui
     */
    void InitializeImGui();

    /**
     * Shutdown ImGui
     */
    void ShutdownImGui();

    /**
     * Handle keyboard shortcuts for application controls
     */
    void HandleKeyboardInput();

private:
    /**
     * Create application specification from pre-loaded config
     * @param config Configuration containing window dimensions
     */
    static Kappa::ApplicationSpecification CreateApplicationSpecification(const PrecisionTuner::Config &config);

    PrecisionTuner::Config config; ///< Application configuration

    // Layer references for keyboard shortcuts (owned by layer stack)
    PrecisionTuner::Layers::AudioProcessingLayer *audioLayer;
    PrecisionTuner::Layers::TunerVisualizationLayer *tunerLayer;
    PrecisionTuner::Layers::SettingsLayer *settingsLayer;
};
