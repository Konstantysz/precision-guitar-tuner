#pragma once

#include <span>
#include <string>
#include <AudioDevice.h>

/**
 * @brief Mock implementation of AudioDevice for testing
 *
 * Provides a controllable audio device for unit testing audio processing
 * components without requiring real hardware. Supports simulating device
 * operations, callback triggering, and error conditions.
 */
class MockAudioDevice : public GuitarIO::AudioDevice
{
public:
    MockAudioDevice();

    /**
     * @brief Opens the mock audio device
     * @param deviceId Device identifier (ignored in mock)
     * @param config Audio stream configuration
     * @param userCallback Callback function for audio processing
     * @param userPtr User data pointer passed to callback
     * @return Result controlled by SetOpenResult() (default: true)
     */
    bool Open(uint32_t deviceId,
        const GuitarIO::AudioStreamConfig &config,
        GuitarIO::AudioCallback userCallback,
        void *userPtr = nullptr) override;

    /**
     * @brief Opens the default mock audio device
     * @param config Audio stream configuration
     * @param userCallback Callback function for audio processing
     * @param userPtr User data pointer passed to callback
     * @return Result controlled by SetOpenResult() (default: true)
     */
    bool OpenDefault(const GuitarIO::AudioStreamConfig &config,
        GuitarIO::AudioCallback userCallback,
        void *userPtr = nullptr) override;

    /**
     * @brief Starts the mock audio stream
     * @return Result controlled by SetStartResult() (default: true), or false if not open
     */
    bool Start() override;

    /**
     * @brief Stops the mock audio stream
     * @return Always returns true
     */
    bool Stop() override;

    /**
     * @brief Closes the mock audio device
     */
    void Close() override;

    /**
     * @brief Checks if the mock device is open
     * @return true if Open() has been called, false otherwise
     */
    bool IsOpen() const override;

    /**
     * @brief Checks if the mock device is running
     * @return true if Start() has been called, false otherwise
     */
    bool IsRunning() const override;

    /**
     * @brief Gets the last error message
     * @return Always returns "Mock Error"
     */
    std::string GetLastError() const override;

    // Test Helper Methods

    /**
     * @brief Sets the result that Open() will return
     * @param result Result value (true = success, false = failure)
     */
    void SetOpenResult(bool result);

    /**
     * @brief Sets the result that Start() will return
     * @param result Result value (true = success, false = failure)
     */
    void SetStartResult(bool result);

    /**
     * @brief Manually triggers the audio callback for testing
     * @param input Input audio buffer
     * @param output Output audio buffer
     * @return Callback return value, or 0 if callback is null or device not running
     */
    int TriggerCallback(std::span<const float> input, std::span<float> output);

    /**
     * @brief Gets the current stream configuration
     * @return Reference to the stored configuration
     */
    const GuitarIO::AudioStreamConfig &GetConfig() const;

private:
    bool isOpen;
    bool isRunning;
    bool openResult;
    bool startResult;

    GuitarIO::AudioStreamConfig config;
    GuitarIO::AudioCallback callback;
    void *userPtr;
};
