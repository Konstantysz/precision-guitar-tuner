#pragma once

#include "../external/lib-guitar-io/include/AudioMixer.h"
#include "../external/lib-guitar-io/include/PolyphonicGenerator.h"
#include "../external/lib-guitar-io/include/SineWaveGenerator.h"
#include <Layer.h>
#include <atomic>
#include <memory>
#include <span>
#include <vector>
#include <AudioDevice.h>
#include <AudioDeviceManager.h>
#include <Config.h>
#include <HybridPitchDetector.h>
#include <PitchStabilizer.h>

namespace PrecisionTuner::Layers
{

    /**
     * @brief Layer responsible for audio I/O and real-time pitch detection
     *
     * This layer manages the audio callback thread and performs pitch detection
     * using the YIN algorithm. It follows real-time audio constraints:
     * - No allocations in audio callback
     * - Lock‑free communication with UI thread
     * - Pre‑allocated buffers
     */
    class AudioProcessingLayer : public Kappa::Layer
    {
    public:
        /** Pitch stabilization algorithm types */
        enum class StabilizerType
        {
            None,   ///< No stabilization (raw YIN output)
            EMA,    ///< Exponential Moving Average
            Median, ///< Median filter
            Hybrid  ///< Hybrid (median + confidence-weighted EMA) - recommended
        };

        /** Configuration for the audio processing layer */
        struct Config
        {
            uint32_t sampleRate = 48000;  ///< Sample rate (Hz)
            uint32_t bufferSize = 2048;   ///< Buffer size (frames) – larger for better pitch accuracy
            float minFrequency = 80.0f;   ///< Minimum detectable frequency (E2)
            float maxFrequency = 1200.0f; ///< Maximum detectable frequency (D6)

            // Pitch stabilization configuration
            StabilizerType stabilizerType = StabilizerType::Hybrid; ///< Stabilization algorithm
            float emaAlpha = 0.3f;                                  ///< EMA smoothing factor [0.0, 1.0]
            uint32_t medianWindowSize = 5;                          ///< Median filter window size
        };

        /** Result of pitch detection (lock‑free) */
        struct PitchData
        {
            float frequency = 0.0f;  ///< Detected frequency in Hz
            float confidence = 0.0f; ///< Detection confidence [0.0, 1.0]
            bool detected = false;   ///< Whether a pitch was detected
        };

        /**
         * @brief Constructs the audio processing layer
         * @param config Layer configuration
         */
        explicit AudioProcessingLayer(const Config &config = Config{});

        ~AudioProcessingLayer() override;

        void OnUpdate(float deltaTime) override;

        /**
         * @brief Gets the latest detected pitch data
         * Thread-safe method to retrieve pitch information from the audio callback.
         * @return Latest pitch data (frequency, confidence, detection status)
         */
        [[nodiscard]] PitchData GetLatestPitch() const;

        /**
         * @brief Checks if audio processing is active
         * @return true if audio stream is running, false otherwise
         */
        [[nodiscard]] bool IsRunning() const;

        // Input device methods

        /**
         * @brief Gets a list of available input device names
         * @return Vector of device names
         */
        [[nodiscard]] std::vector<std::string> GetAvailableInputDevices() const;

        /**
         * @brief Gets detailed information for all available input devices
         * @return Vector of audio device info structures
         */
        [[nodiscard]] std::vector<GuitarIO::AudioDeviceInfo> GetAvailableInputDeviceInfo() const;

        /**
         * @brief Gets the ID of the currently active input device
         * @return Device ID
         */
        [[nodiscard]] uint32_t GetCurrentInputDeviceId() const;

        /**
         * @brief Switches the active input device
         * @param deviceId ID of the device to switch to
         * @return true if switch was successful, false otherwise
         */
        bool SwitchInputDevice(uint32_t deviceId);

        /**
         * @brief Gets a list of available output device names
         * @return Vector of device names
         */
        [[nodiscard]] std::vector<std::string> GetAvailableOutputDevices() const;

        /**
         * @brief Gets detailed information for all available output devices
         * @return Vector of audio device info structures
         */
        [[nodiscard]] std::vector<GuitarIO::AudioDeviceInfo> GetAvailableOutputDeviceInfo() const;

        /**
         * @brief Gets the ID of the currently active output device
         * @return Device ID
         */
        [[nodiscard]] uint32_t GetCurrentOutputDeviceId() const;

        /**
         * @brief Switches the active output device
         * @param deviceId ID of the device to switch to
         * @return true if switch was successful, false otherwise
         */
        bool SwitchOutputDevice(uint32_t deviceId);

        /**
         * @brief Updates audio feedback settings
         * Applies changes to beep, reference tone, and monitoring parameters.
         * @param audioCfg New audio configuration
         */
        void UpdateAudioFeedback(const PrecisionTuner::AudioConfig &audioCfg);

        /**
         * @brief Sets frequencies for polyphonic chord playback
         * @param frequencies Array of 6 frequencies (Hz), 0 = disabled voice
         */
        void SetPolyphonicFrequencies(const std::array<float, 6> &frequencies);

        /**
         * @brief Gets the current input signal level
         * @return RMS level of the input signal (0.0 to 1.0)
         */
        [[nodiscard]] float GetInputLevel() const;

    private:
        /**
         * @brief Audio input callback
         * Processes incoming audio for pitch detection and monitoring.
         * @param inputBuffer Input audio samples
         * @param outputBuffer Output audio samples (unused for input callback)
         * @param userData Pointer to AudioProcessingLayer instance
         * @return 0 to continue
         */
        static int InputCallback(std::span<const float> inputBuffer, std::span<float> outputBuffer, void *userData);

        /**
         * @brief Audio output callback
         * Generates audio feedback (beeps, reference tones).
         * @param inputBuffer Input audio samples (unused for output callback)
         * @param outputBuffer Output audio samples to fill
         * @param userData Pointer to AudioProcessingLayer instance
         * @return 0 to continue
         */
        static int OutputCallback(std::span<const float> inputBuffer, std::span<float> outputBuffer, void *userData);

        /**
         * @brief Processes input audio for pitch detection
         * Runs the pitch detection algorithm on the provided buffer.
         * @param inputBuffer Audio samples to process
         */
        void ProcessAudio(std::span<const float> inputBuffer);

        /**
         * @brief Mixes audio feedback into the output buffer
         * Adds beep, reference tone, and monitoring signal to the output.
         * @param outputBuffer Buffer to mix audio into
         */
        void MixFeedback(std::span<float> outputBuffer);

        Config config;                                                 ///< Layer configuration
        std::unique_ptr<GuitarIO::AudioDevice> inputDevice;            ///< Audio input device
        std::unique_ptr<GuitarIO::AudioDevice> outputDevice;           ///< Audio output device
        std::unique_ptr<GuitarDSP::HybridPitchDetector> pitchDetector; ///< Pitch detection algorithm
        std::unique_ptr<GuitarDSP::PitchStabilizer> pitchStabilizer;   ///< Pitch stabilization filter

        // Lock‑free communication
        std::atomic<float> latestFrequency{ 0.0f };  ///< Latest detected frequency (Hz)
        std::atomic<float> latestConfidence{ 0.0f }; ///< Latest detection confidence [0.0, 1.0]
        std::atomic<bool> pitchDetected{ false };    ///< Whether pitch was detected in last frame

        // Pre‑allocated processing buffer
        std::vector<float> processingBuffer;    ///< Buffer for DSP processing
        std::vector<float> outputScratchBuffer; ///< Temporary buffer for output mixing

        // Device tracking
        uint32_t currentInputDeviceId = static_cast<uint32_t>(-1);  ///< Active input device ID
        uint32_t currentOutputDeviceId = static_cast<uint32_t>(-1); ///< Active output device ID
        uint32_t outputChannels = 1;                                ///< Number of output channels

        // Ring buffer for input monitoring
        std::vector<float> monitoringRingBuffer;     ///< Ring buffer for audio pass-through
        std::atomic<size_t> monitoringWritePos{ 0 }; ///< Write position in ring buffer
        std::atomic<size_t> monitoringReadPos{ 0 };  ///< Read position in ring buffer

        // Audio feedback generators and state
        GuitarIO::SineWaveGenerator beepGenerator{ static_cast<double>(config.sampleRate) }; ///< Beep generator
        GuitarIO::SineWaveGenerator referenceGenerator{ static_cast<double>(
            config.sampleRate) }; ///< Reference tone generator
        GuitarIO::PolyphonicGenerator polyphonicGenerator{ static_cast<double>(
            config.sampleRate) }; ///< Polyphonic generator

        std::atomic<bool> beepEnabled{ false };            ///< Beep feedback enabled
        std::atomic<bool> referenceEnabled{ false };       ///< Reference tone enabled
        std::atomic<bool> inputMonitoringEnabled{ false }; ///< Input monitoring enabled
        std::atomic<bool> droneEnabled{ false };           ///< Drone mode enabled
        std::atomic<bool> polyphonicEnabled{ false };      ///< Polyphonic mode enabled

        std::atomic<float> beepVolume{ 0.5f };           ///< Beep volume
        std::atomic<float> referenceVolume{ 0.5f };      ///< Reference tone volume
        std::atomic<float> monitoringVolume{ 0.5f };     ///< Monitoring volume
        std::atomic<float> inputGain{ 1.0f };            ///< Input signal gain
        std::atomic<float> referenceFrequency{ 440.0f }; ///< Reference frequency
        std::atomic<float> currentInputLevel{ 0.0f };    ///< Current input RMS level
    };

} // namespace PrecisionTuner::Layers
