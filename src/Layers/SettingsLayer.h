#pragma once

#include <Layer.h>
#include <memory>
#include <vector>
#include <AudioDeviceManager.h>
#include <AudioProcessingLayer.h>
#include <Config.h>
#include <TunerVisualizationLayer.h>

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
        SettingsLayer(AudioProcessingLayer &audioLayer,
            TunerVisualizationLayer &tunerLayer,
            PrecisionTuner::Config &config);

        ~SettingsLayer() override;

        void OnUpdate(float deltaTime) override;

        void OnRender() override;

    private:
        /**
         * @brief Renders input audio device selection dropdown
         */
        void RenderInputDeviceSelector();

        /**
         * @brief Renders output audio device selection dropdown
         */
        void RenderOutputDeviceSelector();

        /**
         * @brief Renders reference pitch adjustment slider (A=430-450 Hz)
         */
        void RenderReferencePitchSlider();

        /**
         * @brief Renders tuning mode selector
         */
        void RenderTuningModeSelector();

        /**
         * @brief Renders audio feedback controls (beep, reference tone, monitoring)
         */
        void RenderAudioFeedbackControls();

        AudioProcessingLayer &audioLayer;    ///< Reference to audio processing layer
        TunerVisualizationLayer &tunerLayer; ///< Reference to tuner visualization layer
        PrecisionTuner::Config &config;      ///< Reference to application configuration

        // UI state
        bool showSettings; ///< Visibility state of settings window

        // Input device selection
        int selectedInputDeviceIndex;                                 ///< Currently selected input device index
        std::vector<GuitarIO::AudioDeviceInfo> availableInputDevices; ///< List of available input devices

        // Output device selection
        int selectedOutputDeviceIndex;                                 ///< Currently selected output device index
        std::vector<GuitarIO::AudioDeviceInfo> availableOutputDevices; ///< List of available output devices
    };

} // namespace PrecisionTuner::Layers
