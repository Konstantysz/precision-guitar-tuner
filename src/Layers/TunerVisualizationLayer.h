#pragma once

#include "AudioProcessingLayer.h"
#include "FontRenderer.h"
#include <Layer.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <NoteConverter.h>

namespace PrecisionTuner::Layers
{

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
    // Helper methods for rendering
    void InitializeOpenGL();
    void RenderBackground();
    void RenderCentDeviationMeter();
    void RenderTuningIndicator();
    glm::vec3 GetColorForCents(float cents);
    void DrawFilledRect(float x, float y, float width, float height, const glm::vec3 &color);
    void DrawOutlineRect(float x, float y, float width, float height, const glm::vec3 &color, float lineWidth = 2.0f);
    void DrawCircle(float x, float y, float radius, const glm::vec3 &color, bool filled = true);
    void SetupShaders();

    AudioProcessingLayer &audioLayer;

    // OpenGL resources
    bool initialized = false;
    GLuint geometryShaderProgram = 0;  // For shapes (circles, rects)
    GLuint textShaderProgram = 0;       // For text rendering
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint textVAO = 0;
    GLuint textVBO = 0;

    std::unique_ptr<FontRenderer> fontRenderer;

    // UI state
    GuitarDSP::NoteInfo currentNote;
    float updateTimer = 0.0f;
    bool hasPitchData = false;
    static constexpr float UPDATE_INTERVAL = 0.1f; // Update UI every 100ms

    // Visual constants
    static constexpr float METER_WIDTH = 0.8f;       // 80% of screen width
    static constexpr float METER_HEIGHT = 0.05f;     // 5% of screen height
    static constexpr float INDICATOR_RADIUS = 0.08f; // Circular indicator radius
};

} // namespace PrecisionTuner::Layers
