#pragma once

#include <AudioProcessingLayer.h>
#include <Layer.h>
#include <NoteConverter.h>
#include <imgui.h>

namespace PrecisionTuner::Layers
{

/**
 * @brief Layer responsible for rendering the tuner UI using ImGui
 *
 * Displays:
 * - Current detected note and frequency
 * - Cent deviation from target pitch (-50 to +50 range)
 * - Visual tuning indicator with color coding
 * - IN TUNE indicator when within ±3 cents
 */
class TunerVisualizationLayer : public Kappa::Layer
{
public:
    /**
     * @brief Constructs tuner visualization layer
     * @param audioLayer Reference to audio processing layer
     */
    explicit TunerVisualizationLayer(AudioProcessingLayer &audioLayer);

    /**
     * @brief Destructor
     */
    ~TunerVisualizationLayer() override = default;

    /**
     * @brief Called every frame to update layer state
     * @param deltaTime Time since last frame (seconds)
     */
    void OnUpdate(float deltaTime) override;

    /**
     * @brief Called every frame to render the tuner UI
     */
    void OnRender() override;

private:
    /**
     * @brief Renders the cent deviation meter (-50 to +50 range)
     */
    void RenderCentDeviationMeter();

    /**
     * @brief Renders the tuning indicator (note name, frequency, cents)
     */
    void RenderTuningIndicator();

    /**
     * @brief Gets color for given cent deviation
     * @param cents Cent deviation from target
     * @return RGB color (green = in-tune, yellow = close, red = far)
     */
    ImVec4 GetColorForCents(float cents);

    AudioProcessingLayer &audioLayer;

    // UI state
    GuitarDSP::NoteInfo currentNote;
    float updateTimer = 0.0f;
    bool hasPitchData = false;
    static constexpr float UPDATE_INTERVAL = 0.1f; // Update UI every 100ms
};

} // namespace PrecisionTuner::Layers
