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

        /**
         * @brief Destructor
         */
        ~TunerVisualizationLayer() override;

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
         * @brief Renders the retro gauge visualization
         */
        void RenderRetroGauge();

        /**
         * @brief Get color based on cent deviation
         * @param cents Deviation from target in cents
         * @return Color (green=in-tune, yellow=close, orange/red=far)
         */
        ImVec4 GetColorForCents(float cents);

        /**
         * @brief Load a texture from file
         * @param path Path to texture file
         * @return ImGui texture ID (0 if failed)
         */
        ImTextureID LoadTexture(const char *path);

        /**
         * @brief Initialize all gauge textures
         */
        void InitializeTextures();

        /**
         * @brief Cleanup loaded textures
         */
        void CleanupTextures();

        AudioProcessingLayer &audioLayer; ///< Reference to audio processing layer
        PrecisionTuner::Config &config;   ///< Reference to application configuration

        std::optional<GuitarDSP::NoteInfo> currentNote; ///< Currently detected note information
        float updateTimer;                              ///< Timer for UI updates
        bool hasPitchData;                              ///< Flag indicating if valid pitch data is available
        bool showSettingsPanel;                         ///< Visibility state of settings panel
        std::optional<int> targetStringIndex;           ///< Active string index (0=6th, 5=1st)

        float smoothedCents; ///< Smoothed cent deviation for display

        // Texture IDs for visual assets
        ImTextureID woodBackgroundTexture; ///< Wood background texture
        ImTextureID gaugeFaceTexture;      ///< Cream gauge face texture
        ImTextureID chromeTexture;         ///< Chrome bezel texture
    };

} // namespace PrecisionTuner::Layers
