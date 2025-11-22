#pragma once

#include "AudioProcessingLayer.h"
#include <Layer.h>
#include <memory>
#include <NoteConverter.h>

/**
 * @brief Layer responsible for rendering the tuner UI
 *
 * Displays:
 * - Current detected note and frequency
 * - Cent deviation from target pitch
 * - Visual tuning indicator
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
     * @brief Called every frame to render the layer
     */
    void OnRender() override;

private:
    AudioProcessingLayer &audioLayer;

    // UI state
    GuitarDSP::NoteInfo currentNote;
    float updateTimer = 0.0f;
    static constexpr float UPDATE_INTERVAL = 0.1f; // Update UI every 100ms
};
