#include "AudioProcessingLayer.h"
#include <AudioDeviceManager.h>
#include <Logger.h>

AudioProcessingLayer::AudioProcessingLayer(const Config& config)
    : config_(config)
    , audioDevice_(std::make_unique<GuitarIO::AudioDevice>())
    , pitchDetector_(std::make_unique<GuitarDSP::YinPitchDetector>(
        GuitarDSP::YinPitchDetector::Config{
            .threshold = 0.15f,
            .minFrequency = config.minFrequency,
            .maxFrequency = config.maxFrequency
        }))
{
    // Pre-allocate processing buffer (avoid allocations in audio callback)
    processingBuffer_.resize(config_.bufferSize);

    LOG_INFO("AudioProcessingLayer - Initializing audio I/O");

    // Configure audio stream
    GuitarIO::AudioStreamConfig streamConfig{
        .sampleRate = config_.sampleRate,
        .bufferSize = config_.bufferSize,
        .inputChannels = 1,  // Mono input for guitar
        .outputChannels = 0  // No output (input-only)
    };

    // Open default audio input device
    if (!audioDevice_->OpenDefault(streamConfig, AudioCallback, this))
    {
        LOG_ERROR("Failed to open audio device: {}", audioDevice_->GetLastError());
        return;
    }

    // Start audio stream
    if (!audioDevice_->Start())
    {
        LOG_ERROR("Failed to start audio stream: {}", audioDevice_->GetLastError());
        return;
    }

    LOG_INFO("Audio stream started successfully");
    LOG_INFO("  Sample Rate: {} Hz", config_.sampleRate);
    LOG_INFO("  Buffer Size: {} frames", config_.bufferSize);
    LOG_INFO("  Frequency Range: {:.1f} - {:.1f} Hz", config_.minFrequency, config_.maxFrequency);
}

AudioProcessingLayer::~AudioProcessingLayer()
{
    if (audioDevice_->IsRunning())
    {
        LOG_INFO("AudioProcessingLayer - Stopping audio stream");
        audioDevice_->Stop();
    }

    if (audioDevice_->IsOpen())
    {
        audioDevice_->Close();
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
    data.detected = pitchDetected_.load(std::memory_order_relaxed);
    data.frequency = latestFrequency_.load(std::memory_order_relaxed);
    data.confidence = latestConfidence_.load(std::memory_order_relaxed);
    return data;
}

bool AudioProcessingLayer::IsRunning() const
{
    return audioDevice_->IsRunning();
}

std::vector<std::string> AudioProcessingLayer::GetAvailableDevices() const
{
    auto& manager = GuitarIO::AudioDeviceManager::Get();
    auto devices = manager.EnumerateInputDevices();

    std::vector<std::string> deviceNames;
    deviceNames.reserve(devices.size());

    for (const auto& device : devices)
    {
        deviceNames.push_back(device.name);
    }

    return deviceNames;
}

int AudioProcessingLayer::AudioCallback(const float* inputBuffer,
                                       [[maybe_unused]] float* outputBuffer,
                                       size_t frameCount,
                                       void* userData)
{
    auto* layer = static_cast<AudioProcessingLayer*>(userData);
    if (!layer || !inputBuffer)
    {
        return 1; // Stop stream
    }

    // Process audio in real-time thread
    layer->ProcessAudio(inputBuffer, frameCount);

    return 0; // Continue stream
}

void AudioProcessingLayer::ProcessAudio(const float* inputBuffer, size_t frameCount)
{
    // Detect pitch using YIN algorithm
    auto result = pitchDetector_->Detect(inputBuffer, frameCount, static_cast<float>(config_.sampleRate));

    if (result.has_value())
    {
        // Update atomic variables (lock-free communication with UI thread)
        latestFrequency_.store(result->frequency, std::memory_order_relaxed);
        latestConfidence_.store(result->confidence, std::memory_order_relaxed);
        pitchDetected_.store(true, std::memory_order_relaxed);
    }
    else
    {
        // No pitch detected
        pitchDetected_.store(false, std::memory_order_relaxed);
    }
}
