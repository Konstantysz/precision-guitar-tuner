#pragma once

#include "../external/lib-guitar-io/include/AudioMixer.h"
#include "../external/lib-guitar-io/include/SineWaveGenerator.h"
#include <Layer.h>
#include <atomic>
#include <memory>
#include <span>
#include <vector>
#include <AudioDevice.h>
#include <AudioDeviceManager.h>
#include <Config.h>
#include <YinPitchDetector.h>

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
        /** Configuration for the audio processing layer */
        struct Config
        {
            uint32_t sampleRate = 48000;  ///< Sample rate (Hz)
            uint32_t bufferSize = 2048;   ///< Buffer size (frames) – larger for better pitch accuracy
            float minFrequency = 80.0f;   ///< Minimum detectable frequency (E2)
            float maxFrequency = 1200.0f; ///< Maximum detectable frequency (D6)
        };

        /** Result of pitch detection (lock‑free) */
        struct PitchData
        {
            float frequency = 0.0f;  ///< Detected frequency in Hz
            float confidence = 0.0f; ///< Detection confidence [0.0, 1.0]
            bool detected = false;   ///< Whether a pitch was detected
        };

        explicit AudioProcessingLayer(const Config &config = Config{});
        ~AudioProcessingLayer() override;

        void OnUpdate(float deltaTime) override;

        [[nodiscard]] PitchData GetLatestPitch() const;
        [[nodiscard]] bool IsRunning() const;
        // Input device methods
        [[nodiscard]] std::vector<std::string> GetAvailableInputDevices() const;
        [[nodiscard]] std::vector<GuitarIO::AudioDeviceInfo> GetAvailableInputDeviceInfo() const;
        [[nodiscard]] uint32_t GetCurrentInputDeviceId() const;
        bool SwitchInputDevice(uint32_t deviceId);

        // Output device methods
        [[nodiscard]] std::vector<std::string> GetAvailableOutputDevices() const;
        [[nodiscard]] std::vector<GuitarIO::AudioDeviceInfo> GetAvailableOutputDeviceInfo() const;
        [[nodiscard]] uint32_t GetCurrentOutputDeviceId() const;
        bool SwitchOutputDevice(uint32_t deviceId);

        void UpdateAudioFeedback(const PrecisionTuner::AudioConfig &audioCfg);

    private:
        static int InputCallback(std::span<const float> inputBuffer, std::span<float> outputBuffer, void *userData);
        static int OutputCallback(std::span<const float> inputBuffer, std::span<float> outputBuffer, void *userData);
        void ProcessAudio(std::span<const float> inputBuffer);
        void MixFeedback(std::span<float> outputBuffer);

        Config config;
        std::unique_ptr<GuitarIO::AudioDevice> inputDevice;
        std::unique_ptr<GuitarIO::AudioDevice> outputDevice;
        std::unique_ptr<GuitarDSP::YinPitchDetector> pitchDetector;

        // Lock‑free communication
        std::atomic<float> latestFrequency{ 0.0f };
        std::atomic<float> latestConfidence{ 0.0f };
        std::atomic<bool> pitchDetected{ false };

        // Pre‑allocated processing buffer
        std::vector<float> processingBuffer;
        std::vector<float> outputScratchBuffer;

        // Device tracking
        uint32_t currentInputDeviceId = static_cast<uint32_t>(-1);
        uint32_t currentOutputDeviceId = static_cast<uint32_t>(-1);
        uint32_t outputChannels = 1;

        // Ring buffer for input monitoring
        std::vector<float> monitoringRingBuffer;
        std::atomic<size_t> monitoringWritePos{ 0 };
        std::atomic<size_t> monitoringReadPos{ 0 };

        // Audio feedback generators and state
        GuitarIO::SineWaveGenerator beepGenerator{ static_cast<double>(config.sampleRate) };
        GuitarIO::SineWaveGenerator referenceGenerator{ static_cast<double>(config.sampleRate) };

        std::atomic<bool> beepEnabled{ false };
        std::atomic<bool> referenceEnabled{ false };
        std::atomic<bool> inputMonitoringEnabled{ false };

        std::atomic<float> beepVolume{ 0.5f };
        std::atomic<float> referenceVolume{ 0.5f };
        std::atomic<float> monitoringVolume{ 0.5f };
        std::atomic<float> inputGain{ 1.0f };
        std::atomic<float> referenceFrequency{ 440.0f };
    };

} // namespace PrecisionTuner::Layers
