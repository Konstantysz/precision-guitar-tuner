#pragma once

#include <Layer.h>
#include <memory>
#include <vector>
#include <AudioDeviceManager.h>
#include <AudioProcessingLayer.h>
#include <TunerVisualizationLayer.h>
#include <Config.h>

namespace PrecisionTuner::Layers
{

/**
 * @brief Layer responsible for application settings UI
 *
 * Provides ImGui-based settings interface for:
 * - Audio device selection
 * - Reference pitch adjustment (A=430-450 Hz)
 * - Tuning mode selection (future)
 *
 * Settings are persisted via Config system and saved on application shutdown.
 */
class SettingsLayer : public Kappa::Layer
{
public:
    /**
     * @brief Constructs settings layer
     * @param audioLayer Reference to audio processing layer (for device switching)
     * @param tunerLayer Reference to tuner visualization layer (for visibility control)
     * @param config Reference to application config (for persistence)
     */
    SettingsLayer(AudioProcessingLayer &audioLayer, TunerVisualizationLayer &tunerLayer, PrecisionTuner::Config &config);

    /**
     * @brief Destructor - cleans up ImGui context
     */
    ~SettingsLayer() override;

    /**
     * @brief Called every frame to update layer state
     * @param deltaTime Time since last frame (seconds)
     */
    void OnUpdate(float deltaTime) override;

    /**
     * @brief Called every frame to render UI
     */
    void OnRender() override;

private:
    /**
     * @brief Initializes ImGui context and backends
     */
    void InitializeImGui();

    /**
     * @brief Renders audio device selection dropdown
     */
    void RenderDeviceSelector();

    /**
     * @brief Renders reference pitch adjustment slider (A=430-450 Hz)
     */
    void RenderReferencePitchSlider();

    /**
     * @brief Renders tuning mode selector (placeholder for Phase 3)
     */
    void RenderTuningModeSelector();

    AudioProcessingLayer &audioLayer;
    TunerVisualizationLayer &tunerLayer;
    PrecisionTuner::Config &config;

    // UI state
    bool showSettings = true;
    int selectedDeviceIndex = 0;
    std::vector<GuitarIO::AudioDeviceInfo> availableDevices;
    bool imguiInitialized = false;
};

} // namespace PrecisionTuner::Layers
