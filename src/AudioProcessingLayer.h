#pragma once

#include <Layer.h>
#include <atomic>
#include <memory>
#include <vector>
#include <AudioDevice.h>
#include <YinPitchDetector.h>

/**
 * @brief Layer responsible for audio I/O and real-time pitch detection
 *
 * This layer manages the audio callback thread and performs pitch detection
 * using the YIN algorithm. It follows real-time audio constraints:
 * - No allocations in audio callback
 * - Lock-free communication with UI thread
 * - Pre-allocated buffers
 */
class AudioProcessingLayer : public Kappa::Layer
{
public:
    /**
     * @brief Audio processing configuration
     */
    struct Config
    {
        uint32_t sampleRate = 48000;  ///< Sample rate (Hz)
        uint32_t bufferSize = 2048;   ///< Buffer size (frames) - larger for better pitch accuracy
        float minFrequency = 80.0f;   ///< Minimum detectable frequency (E2)
        float maxFrequency = 1200.0f; ///< Maximum detectable frequency (D6)
    };

    /**
     * @brief Pitch detection result (thread-safe, lock-free)
     */
    struct PitchData
    {
        float frequency = 0.0f;  ///< Detected frequency in Hz
        float confidence = 0.0f; ///< Detection confidence [0.0, 1.0]
        bool detected = false;   ///< Whether pitch was detected
    };

    /**
     * @brief Constructs audio processing layer
     * @param config Audio configuration
     */
    explicit AudioProcessingLayer(const Config &config = Config{});

    /**
     * @brief Destructor
     */
    ~AudioProcessingLayer() override;

    /**
     * @brief Called every frame to update layer state
     * @param deltaTime Time since last frame (seconds)
     */
    void OnUpdate(float deltaTime) override;

    /**
     * @brief Returns the latest pitch detection result (thread-safe)
     * @return Pitch data
     */
    [[nodiscard]] PitchData GetLatestPitch() const;

    /**
     * @brief Checks if audio stream is running
     * @return true if running, false otherwise
     */
    [[nodiscard]] bool IsRunning() const;

    /**
     * @brief Returns available audio input devices
     * @return Vector of device names
     */
    [[nodiscard]] std::vector<std::string> GetAvailableDevices() const;

private:
    /**
     * @brief Audio callback (real-time thread)
     * @param inputBuffer Input audio samples
     * @param outputBuffer Output audio samples (unused)
     * @param frameCount Number of frames
     * @param userData User data pointer
     * @return 0 to continue, non-zero to stop
     */
    static int AudioCallback(const float *inputBuffer, float *outputBuffer, size_t frameCount, void *userData);

    /**
     * @brief Processes audio buffer and detects pitch (real-time thread)
     * @param inputBuffer Input audio samples
     * @param frameCount Number of frames
     */
    void ProcessAudio(const float *inputBuffer, size_t frameCount);

    Config config;
    std::unique_ptr<GuitarIO::AudioDevice> audioDevice;
    std::unique_ptr<GuitarDSP::YinPitchDetector> pitchDetector;

    // Lock-free communication between audio thread and UI thread
    std::atomic<float> latestFrequency{ 0.0f };
    std::atomic<float> latestConfidence{ 0.0f };
    std::atomic<bool> pitchDetected{ false };

    // Pre-allocated buffer for audio processing (to avoid allocations in callback)
    std::vector<float> processingBuffer;
};
