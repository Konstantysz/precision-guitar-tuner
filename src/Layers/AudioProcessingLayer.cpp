#include "AudioProcessingLayer.h"
#include <Logger.h>
#include <AudioDeviceManager.h>

namespace PrecisionTuner::Layers
{

AudioProcessingLayer::AudioProcessingLayer(const Config &config)
    : config(config), audioDevice(std::make_unique<GuitarIO::AudioDevice>()),
      pitchDetector(
          std::make_unique<GuitarDSP::YinPitchDetector>(GuitarDSP::YinPitchDetector::Config{ .threshold = 0.15f,
              .minFrequency = config.minFrequency,
              .maxFrequency = config.maxFrequency }))
{
    // Pre-allocate processing buffer (avoid allocations in audio callback)
    processingBuffer.resize(config.bufferSize);

    LOG_INFO("AudioProcessingLayer - Initializing audio I/O");

    // Enumerate and log all available input devices
    auto &deviceManager = GuitarIO::AudioDeviceManager::Get();
    auto inputDevices = deviceManager.EnumerateInputDevices();

    LOG_INFO("Available input devices ({} found):", inputDevices.size());
    for (const auto &device : inputDevices)
    {
        LOG_INFO("  [{}] {} - {} input channels, sample rates: {}-{} Hz",
            device.id,
            device.name,
            device.maxInputChannels,
            device.supportedSampleRates.empty() ? 0 : device.supportedSampleRates.front(),
            device.supportedSampleRates.empty() ? 0 : device.supportedSampleRates.back());
    }

    // Log which device will be used as default
    uint32_t defaultId = deviceManager.GetDefaultInputDevice();
    auto defaultInfo = deviceManager.GetDeviceInfo(defaultId);
    LOG_INFO("Using default input device: [{}] {}", defaultId, defaultInfo.name);

    // Track current device (will be default)
    currentDeviceId = defaultId;

    // Configure audio stream
    GuitarIO::AudioStreamConfig streamConfig{
        .sampleRate = config.sampleRate,
        .bufferSize = config.bufferSize,
        .inputChannels = 1, // Mono input for guitar
        .outputChannels = 0 // No output (input-only)
    };

    // Open default audio input device
    if (!audioDevice->OpenDefault(streamConfig, AudioCallback, this))
    {
        LOG_ERROR("Failed to open audio device: {}", audioDevice->GetLastError());
        return;
    }

    // Start audio stream
    if (!audioDevice->Start())
    {
        LOG_ERROR("Failed to start audio stream: {}", audioDevice->GetLastError());
        return;
    }

    LOG_INFO("Audio stream started successfully");
    LOG_INFO("  Sample Rate: {} Hz", config.sampleRate);
    LOG_INFO("  Buffer Size: {} frames", config.bufferSize);
    LOG_INFO("  Frequency Range: {:.1f} - {:.1f} Hz", config.minFrequency, config.maxFrequency);
    LOG_INFO("  Frequency Range: {:.1f} - {:.1f} Hz", config.minFrequency, config.maxFrequency);

    // Pre-allocate YinPitchDetector internal buffer by running a dummy detection
    // This ensures no allocations happen during the first real audio callback
    std::vector<float> dummyBuffer(config.bufferSize, 0.0f);
    (void)pitchDetector->Detect(dummyBuffer, static_cast<float>(config.sampleRate));
    LOG_INFO("YinPitchDetector initialized and pre-allocated");
}

AudioProcessingLayer::~AudioProcessingLayer()
{
    if (audioDevice->IsRunning())
    {
        LOG_INFO("AudioProcessingLayer - Stopping audio stream");
        audioDevice->Stop();
    }

    if (audioDevice->IsOpen())
    {
        audioDevice->Close();
    }
}

void AudioProcessingLayer::OnUpdate([[maybe_unused]] float deltaTime)
{
    // UI thread update - could log pitch data, trigger events, etc.
    // Keep minimal to avoid blocking main thread
}

AudioProcessingLayer::PitchData AudioProcessingLayer::GetLatestPitch() const
{
    // Lock-free read from atomic variables
    PitchData data;
    data.detected = pitchDetected.load(std::memory_order_relaxed);
    data.frequency = latestFrequency.load(std::memory_order_relaxed);
    data.confidence = latestConfidence.load(std::memory_order_relaxed);
    return data;
}

bool AudioProcessingLayer::IsRunning() const
{
    return audioDevice->IsRunning();
}

std::vector<std::string> AudioProcessingLayer::GetAvailableDevices() const
{
    auto &manager = GuitarIO::AudioDeviceManager::Get();
    auto devices = manager.EnumerateInputDevices();

    std::vector<std::string> deviceNames;
    deviceNames.reserve(devices.size());

    for (const auto &device : devices)
    {
        deviceNames.push_back(device.name);
    }

    return deviceNames;
}

int AudioProcessingLayer::AudioCallback(std::span<const float> inputBuffer,
    [[maybe_unused]] std::span<float> outputBuffer,
    void *userData)
{
    auto *layer = static_cast<AudioProcessingLayer *>(userData);
    if (!layer || inputBuffer.empty())
    {
        return 1; // Stop stream
    }

    // Process audio in real-time thread
    layer->ProcessAudio(inputBuffer);

    return 0; // Continue stream
}

void AudioProcessingLayer::ProcessAudio(std::span<const float> inputBuffer)
{
    // Detect pitch using YIN algorithm
    auto result = pitchDetector->Detect(inputBuffer, static_cast<float>(config.sampleRate));

    if (result.has_value())
    {
        // Update atomic variables (lock-free communication with UI thread)
        latestFrequency.store(result->frequency, std::memory_order_relaxed);
        latestConfidence.store(result->confidence, std::memory_order_relaxed);
        pitchDetected.store(true, std::memory_order_relaxed);
    }
    else
    {
        // No pitch detected
        pitchDetected.store(false, std::memory_order_relaxed);
    }
}

std::vector<GuitarIO::AudioDeviceInfo> AudioProcessingLayer::GetAvailableDeviceInfo() const
{
    auto &manager = GuitarIO::AudioDeviceManager::Get();
    return manager.EnumerateInputDevices();
}

uint32_t AudioProcessingLayer::GetCurrentDeviceId() const
{
    return currentDeviceId;
}

bool AudioProcessingLayer::SwitchDevice(uint32_t deviceId)
{
    LOG_INFO("Switching to audio device ID: {}", deviceId);

    // Stop current stream if running
    if (audioDevice->IsRunning())
    {
        LOG_INFO("Stopping current audio stream...");
        if (!audioDevice->Stop())
        {
            LOG_ERROR("Failed to stop audio stream: {}", audioDevice->GetLastError());
            return false;
        }
    }

    // Close current device if open
    if (audioDevice->IsOpen())
    {
        LOG_INFO("Closing current audio device...");
        audioDevice->Close();
    }

    // Configure audio stream
    GuitarIO::AudioStreamConfig streamConfig{
        .sampleRate = config.sampleRate,
        .bufferSize = config.bufferSize,
        .inputChannels = 1, // Mono input for guitar
        .outputChannels = 0 // No output (input-only)
    };

    // Open new device
    LOG_INFO("Opening new audio device...");
    if (!audioDevice->Open(deviceId, streamConfig, AudioCallback, this))
    {
        LOG_ERROR("Failed to open audio device: {}", audioDevice->GetLastError());

        // Try to reopen default device as fallback
        LOG_WARN("Attempting to reopen default device as fallback...");
        if (audioDevice->OpenDefault(streamConfig, AudioCallback, this))
        {
            audioDevice->Start();
            currentDeviceId = static_cast<uint32_t>(-1);
            LOG_INFO("Fallback to default device successful");
        }
        return false;
    }

    // Start new stream
    LOG_INFO("Starting new audio stream...");
    if (!audioDevice->Start())
    {
        LOG_ERROR("Failed to start audio stream: {}", audioDevice->GetLastError());
        audioDevice->Close();

        // Try to reopen default device as fallback
        LOG_WARN("Attempting to reopen default device as fallback...");
        if (audioDevice->OpenDefault(streamConfig, AudioCallback, this))
        {
            audioDevice->Start();
            currentDeviceId = static_cast<uint32_t>(-1);
            LOG_INFO("Fallback to default device successful");
        }
        return false;
    }

    // Update current device ID
    currentDeviceId = deviceId;

    // Log device info
    auto &manager = GuitarIO::AudioDeviceManager::Get();
    auto deviceInfo = manager.GetDeviceInfo(deviceId);
    LOG_INFO("Successfully switched to device: [{}] {}", deviceId, deviceInfo.name);

    return true;
}

} // namespace PrecisionTuner::Layers
