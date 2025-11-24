#pragma once

#include <Layer.h>
#include <imgui.h>
#include <optional>
#include <AudioProcessingLayer.h>
#include <Config.h>
#include <NoteConverter.h>

namespace PrecisionTuner::Layers
{

    /**
     * @brief Layer responsible for rendering the tuner UI using ImGui
     *
     * Displays:
     * - Current detected note and frequency
     * - Cent deviation from target pitch (±50 cent range)
     * - Visual tuning indicator with color coding (green, yellow, orange, red)
     * - IN TUNE indicator when within ±3 cents
     * - Target string indicator for non-chromatic tuning modes
     */
    class TunerVisualizationLayer : public Kappa::Layer
    {
    public:
        /**
         * @brief Construct tuner visualization layer
         * @param audioLayer Reference to audio processing layer for pitch data
         * @param config Reference to application config for tuning mode
         */
        explicit TunerVisualizationLayer(AudioProcessingLayer &audioLayer, PrecisionTuner::Config &config);

        ~TunerVisualizationLayer() override = default;

        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        /**
         * @brief Gets the settings visibility state
         * @return true if settings should be shown
         */
        [[nodiscard]] bool IsSettingsVisible() const;

        /**
         * @brief Sets the settings visibility state
         * @param visible true to show settings, false to hide
         */
        void SetSettingsVisible(bool visible);

    private:
        /**
         * @brief Renders the cent deviation meter (-50 to +50 range)
         */
        void RenderCentDeviationMeter();
        void RenderTuningIndicator();

        /**
         * @brief Render target string indicator for non-chromatic modes
         * Shows string name and visual position (e.g., "6th String (E2)")
         */
        void RenderTargetStringIndicator();

        /**
         * @brief Get color based on cent deviation
         * @param cents Deviation from target in cents
         * @return Color (green=in-tune, yellow=close, orange/red=far)
         */
        ImVec4 GetColorForCents(float cents);

        AudioProcessingLayer &audioLayer;
        PrecisionTuner::Config &config;

        GuitarDSP::NoteInfo currentNote;
        float updateTimer = 0.0f;
        bool hasPitchData = false;
        bool showSettingsPanel = true;
        std::optional<int> targetStringIndex; ///< Active string index (0=6th, 5=1st)

        static constexpr float UPDATE_INTERVAL = 0.1f; ///< UI update rate (100ms)
    };

} // namespace PrecisionTuner::Layers
