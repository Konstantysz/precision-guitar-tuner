#include "AudioProcessingLayer.h"
#include <Logger.h>
#include <algorithm>
#include <AudioDeviceManager.h>

namespace PrecisionTuner::Layers
{
    AudioProcessingLayer::AudioProcessingLayer(const AudioProcessingLayerConfig &config)
        : config(config), inputDevice(std::make_unique<GuitarIO::AudioDevice>()),
          outputDevice(std::make_unique<GuitarIO::AudioDevice>()),
          pitchDetector(std::make_unique<GuitarDSP::HybridPitchDetector>(
              GuitarDSP::HybridPitchDetectorConfig{ .yinConfidenceThreshold = 0.8f,
                  .enableHarmonicRejection = true,
                  .harmonicTolerance = 0.05f,
                  .yinConfig = { .threshold = 0.10f,
                      .minFrequency = config.minFrequency,
                      .maxFrequency = config.maxFrequency },
                  .mpmConfig = { .threshold = 0.93f,
                      .minFrequency = config.minFrequency,
                      .maxFrequency = config.maxFrequency } })),
          pitchStabilizer(nullptr), latestFrequency(0.0f), latestConfidence(0.0f), pitchDetected(false),
          processingBuffer({}), outputScratchBuffer({}), currentInputDeviceId(static_cast<uint32_t>(-1)),
          currentOutputDeviceId(static_cast<uint32_t>(-1)), outputChannels(1), monitoringRingBuffer({}),
          monitoringWritePos(0), monitoringReadPos(0), beepGenerator(static_cast<double>(config.sampleRate)),
          referenceGenerator(static_cast<double>(config.sampleRate)),
          polyphonicGenerator(static_cast<double>(config.sampleRate)), beepEnabled(false), referenceEnabled(false),
          inputMonitoringEnabled(false), droneEnabled(false), polyphonicEnabled(false), beepVolume(0.5f),
          referenceVolume(0.5f), monitoringVolume(0.5f), inputGain(1.0f), referenceFrequency(440.0f),
          currentInputLevel(0.0f)
    {
        // Pre-allocate processing buffer (avoid allocations in audio callback)
        processingBuffer.resize(config.bufferSize);
        outputScratchBuffer.resize(config.bufferSize);

        // Pre-allocate ring buffer for input monitoring (4x buffer size for safety)
        monitoringRingBuffer.resize(config.bufferSize * 4);

        LOG_INFO("AudioProcessingLayer - Initializing audio I/O");

        auto &deviceManager = GuitarIO::AudioDeviceManager::Get();

        // ===== INPUT DEVICE SETUP =====
        auto inputDevices = deviceManager.EnumerateInputDevices();
        LOG_INFO("Available input devices ({} found):", inputDevices.size());
        for (const auto &device : inputDevices)
        {
            LOG_INFO("  [{}] {} - {} input channels", device.id, device.name, device.maxInputChannels);
        }

        uint32_t defaultInputId = deviceManager.GetDefaultInputDevice();
        auto defaultInputInfo = deviceManager.GetDeviceInfo(defaultInputId);
        LOG_INFO("Using default input device: [{}] {}", defaultInputId, defaultInputInfo.name);
        currentInputDeviceId = defaultInputId;

        // Configure input stream (input-only)
        GuitarIO::AudioStreamConfig inputConfig{
            .sampleRate = config.sampleRate, .bufferSize = config.bufferSize, .inputChannels = 1, .outputChannels = 0
        };

        if (!inputDevice->OpenDefault(inputConfig, InputCallback, this))
        {
            LOG_ERROR("Failed to open input device: {}", inputDevice->GetLastError());
            return;
        }

        if (!inputDevice->Start())
        {
            LOG_ERROR("Failed to start input stream: {}", inputDevice->GetLastError());
            return;
        }

        LOG_INFO("Input stream started successfully");

        // ===== OUTPUT DEVICE SETUP =====
        auto outputDevices = deviceManager.EnumerateOutputDevices();
        LOG_INFO("Available output devices ({} found):", outputDevices.size());
        for (const auto &device : outputDevices)
        {
            LOG_INFO("  [{}] {} - {} output channels", device.id, device.name, device.maxOutputChannels);
        }

        // Configure output stream (output-only)
        GuitarIO::AudioStreamConfig outputConfig{
            .sampleRate = config.sampleRate, .bufferSize = config.bufferSize, .inputChannels = 0, .outputChannels = 1
        };

        bool outputDeviceOpened = false;

        if (!outputDevices.empty())
        {
            // Try to open the first available output device
            for (const auto &device : outputDevices)
            {
                LOG_INFO("Trying to open output device: [{}] {}", device.id, device.name);

                // Prefer stereo if available
                uint32_t channels = (device.maxOutputChannels >= 2) ? 2 : 1;
                this->outputChannels = channels;
                outputConfig.outputChannels = channels;

                if (outputDevice->Open(device.id, outputConfig, OutputCallback, this))
                {
                    if (outputDevice->Start())
                    {
                        currentOutputDeviceId = device.id;
                        outputDeviceOpened = true;
                        LOG_INFO("Successfully opened output device: [{}] {} with {} channels",
                            device.id,
                            device.name,
                            channels);
                        break;
                    }
                    else
                    {
                        LOG_WARN("Failed to start output device [{}] {}: {}",
                            device.id,
                            device.name,
                            outputDevice->GetLastError());
                        outputDevice->Close();
                    }
                }
                else
                {
                    LOG_WARN("Failed to open output device [{}] {}: {}",
                        device.id,
                        device.name,
                        outputDevice->GetLastError());

                    // Fallback to mono if stereo failed
                    if (channels > 1)
                    {
                        LOG_WARN("Retrying with mono output...");
                        outputConfig.outputChannels = 1;
                        this->outputChannels = 1;
                        if (outputDevice->Open(device.id, outputConfig, OutputCallback, this))
                        {
                            if (outputDevice->Start())
                            {
                                currentOutputDeviceId = device.id;
                                outputDeviceOpened = true;
                                LOG_INFO("Successfully opened output device (Mono): [{}] {}", device.id, device.name);
                                break;
                            }
                            outputDevice->Close();
                        }
                    }
                }
            }
        }

        if (!outputDeviceOpened)
        {
            LOG_WARN("No working output device found - audio feedback features will be disabled");
            currentOutputDeviceId = static_cast<uint32_t>(-1);
        }

        LOG_INFO("  Sample Rate: {} Hz", config.sampleRate);
        LOG_INFO("  Buffer Size: {} frames", config.bufferSize);
        LOG_INFO("  Frequency Range: {:.1f} - {:.1f} Hz", config.minFrequency, config.maxFrequency);

        // Pre-allocate HybridPitchDetector internal buffer
        std::vector<float> dummyBuffer(config.bufferSize, 0.0f);
        (void)pitchDetector->Detect(dummyBuffer, static_cast<float>(config.sampleRate));
        LOG_INFO("HybridPitchDetector initialized with YIN+MPM and harmonic rejection");

        // Initialize pitch stabilizer based on configuration
        switch (config.stabilizerType)
        {
        case StabilizerType::EMA:
            pitchStabilizer = std::make_unique<GuitarDSP::ExponentialMovingAverage>(
                GuitarDSP::ExponentialMovingAverageConfig{ .alpha = config.emaAlpha });
            LOG_INFO("Pitch stabilization: EMA (alpha={})", config.emaAlpha);
            break;

        case StabilizerType::Median:
            pitchStabilizer = std::make_unique<GuitarDSP::MedianFilter>(
                GuitarDSP::MedianFilterConfig{ .windowSize = config.medianWindowSize });
            LOG_INFO("Pitch stabilization: Median Filter (window={})", config.medianWindowSize);
            break;

        case StabilizerType::Hybrid:
            pitchStabilizer = std::make_unique<GuitarDSP::HybridStabilizer>(GuitarDSP::HybridStabilizerConfig{
                .baseAlpha = config.emaAlpha, .windowSize = config.medianWindowSize });
            LOG_INFO("Pitch stabilization: Hybrid (alpha={}, window={})", config.emaAlpha, config.medianWindowSize);
            break;

        case StabilizerType::None:
        default:
            pitchStabilizer = nullptr;
            LOG_INFO("Pitch stabilization: Disabled");
            break;
        }
    }

    AudioProcessingLayer::~AudioProcessingLayer()
    {
        if (inputDevice->IsRunning())
        {
            LOG_INFO("AudioProcessingLayer - Stopping input stream");
            inputDevice->Stop();
        }
        if (inputDevice->IsOpen())
        {
            inputDevice->Close();
        }

        if (outputDevice->IsRunning())
        {
            LOG_INFO("AudioProcessingLayer - Stopping output stream");
            outputDevice->Stop();
        }
        if (outputDevice->IsOpen())
        {
            outputDevice->Close();
        }
    }

    void AudioProcessingLayer::OnUpdate([[maybe_unused]] float deltaTime)
    {
        // UI thread update
    }

    AudioProcessingLayer::PitchData AudioProcessingLayer::GetLatestPitch() const
    {
        PitchData data;
        data.detected = pitchDetected.load(std::memory_order_relaxed);
        data.frequency = latestFrequency.load(std::memory_order_relaxed);
        data.confidence = latestConfidence.load(std::memory_order_relaxed);
        return data;
    }

    bool AudioProcessingLayer::IsRunning() const
    {
        return inputDevice->IsRunning() && outputDevice->IsRunning();
    }

    std::vector<std::string> AudioProcessingLayer::GetAvailableInputDevices() const
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

    std::vector<GuitarIO::AudioDeviceInfo> AudioProcessingLayer::GetAvailableInputDeviceInfo() const
    {
        auto &manager = GuitarIO::AudioDeviceManager::Get();
        return manager.EnumerateInputDevices();
    }

    uint32_t AudioProcessingLayer::GetCurrentInputDeviceId() const
    {
        return currentInputDeviceId;
    }

    std::vector<std::string> AudioProcessingLayer::GetAvailableOutputDevices() const
    {
        auto &manager = GuitarIO::AudioDeviceManager::Get();
        auto devices = manager.EnumerateOutputDevices();
        std::vector<std::string> deviceNames;
        deviceNames.reserve(devices.size());
        for (const auto &device : devices)
        {
            deviceNames.push_back(device.name);
        }
        return deviceNames;
    }

    std::vector<GuitarIO::AudioDeviceInfo> AudioProcessingLayer::GetAvailableOutputDeviceInfo() const
    {
        auto &manager = GuitarIO::AudioDeviceManager::Get();
        return manager.EnumerateOutputDevices();
    }

    uint32_t AudioProcessingLayer::GetCurrentOutputDeviceId() const
    {
        return currentOutputDeviceId;
    }

    int AudioProcessingLayer::InputCallback(std::span<const float> inputBuffer,
        [[maybe_unused]] std::span<float> outputBuffer,
        void *userData)
    {
        auto *layer = static_cast<AudioProcessingLayer *>(userData);
        if (!layer || inputBuffer.empty())
        {
            return 1; // Stop stream
        }

        // Apply input gain and copy to processing buffer
        float gain = layer->inputGain.load(std::memory_order_relaxed);

        // Resize processing buffer if needed (should already be sized correctly)
        if (layer->processingBuffer.size() < inputBuffer.size())
        {
            layer->processingBuffer.resize(inputBuffer.size());
        }

        for (size_t i = 0; i < inputBuffer.size(); ++i)
        {
            layer->processingBuffer[i] = inputBuffer[i] * gain;
        }

        std::span<const float> gainedBuffer(layer->processingBuffer.data(), inputBuffer.size());

        // Process audio (pitch detection) with gained signal
        layer->ProcessAudio(gainedBuffer);

        // Write to ring buffer for input monitoring (with gain applied)
        if (layer->inputMonitoringEnabled.load(std::memory_order_relaxed))
        {
            size_t writePos = layer->monitoringWritePos.load(std::memory_order_relaxed);
            size_t bufferSize = layer->monitoringRingBuffer.size();

            for (const float sample : gainedBuffer)
            {
                layer->monitoringRingBuffer[writePos] = sample;
                writePos = (writePos + 1) % bufferSize;
            }

            layer->monitoringWritePos.store(writePos, std::memory_order_release);
        }

        // Calculate peak level for metering
        float maxVal = 0.0f;
        for (float sample : gainedBuffer)
        {
            float absVal = std::abs(sample);
            if (absVal > maxVal)
            {
                maxVal = absVal;
            }
        }
        layer->currentInputLevel.store(maxVal, std::memory_order_relaxed);

        return 0; // Continue stream
    }

    int AudioProcessingLayer::OutputCallback([[maybe_unused]] std::span<const float> inputBuffer,
        std::span<float> outputBuffer,
        void *userData)
    {
        auto *layer = static_cast<AudioProcessingLayer *>(userData);
        if (!layer || outputBuffer.empty())
        {
            return 1; // Stop stream
        }

        // Mix feedback audio
        layer->MixFeedback(outputBuffer);

        return 0; // Continue stream
    }

    void AudioProcessingLayer::ProcessAudio(std::span<const float> inputBuffer)
    {
        // Detect pitch using YIN algorithm
        auto result = pitchDetector->Detect(inputBuffer, static_cast<float>(config.sampleRate));

        if (result.has_value())
        {
            GuitarDSP::PitchResult stabilized = result.value();

            // Apply stabilization if enabled
            if (pitchStabilizer)
            {
                pitchStabilizer->Update(result.value());
                stabilized = pitchStabilizer->GetStabilized();
            }

            latestFrequency.store(stabilized.frequency, std::memory_order_relaxed);
            latestConfidence.store(stabilized.confidence, std::memory_order_relaxed);
            pitchDetected.store(true, std::memory_order_relaxed);
        }
        else
        {
            pitchDetected.store(false, std::memory_order_relaxed);
        }
    }

    void AudioProcessingLayer::MixFeedback(std::span<float> outputBuffer)
    {
        if (outputBuffer.empty())
        {
            return;
        }

        // Clear output buffer
        GuitarIO::AudioMixer::Clear(outputBuffer);

        size_t frames = outputBuffer.size() / outputChannels;

        // Safety check for scratch buffer
        if (frames > outputScratchBuffer.size())
        {
            frames = outputScratchBuffer.size();
        }

        // Mix input monitoring from ring buffer
        if (inputMonitoringEnabled.load(std::memory_order_relaxed))
        {
            size_t readPos = monitoringReadPos.load(std::memory_order_acquire);
            size_t writePos = monitoringWritePos.load(std::memory_order_relaxed);
            size_t bufferSize = monitoringRingBuffer.size();

            // Calculate available samples
            size_t available = (writePos >= readPos) ? (writePos - readPos) : (bufferSize - readPos + writePos);
            size_t samplesToRead = std::min(available, frames);

            float vol = monitoringVolume.load(std::memory_order_relaxed);

            for (size_t i = 0; i < samplesToRead; ++i)
            {
                float sample = monitoringRingBuffer[readPos] * vol;

                if (outputChannels == 1)
                {
                    outputBuffer[i] += sample;
                }
                else if (outputChannels == 2)
                {
                    outputBuffer[i * 2] += sample;     // Left
                    outputBuffer[i * 2 + 1] += sample; // Right
                }

                readPos = (readPos + 1) % bufferSize;
            }

            monitoringReadPos.store(readPos, std::memory_order_release);
        }

        // Mix drone mode (continuous reference tone) - takes priority over single reference
        bool droneMode = droneEnabled.load(std::memory_order_relaxed);
        if (droneMode)
        {
            referenceGenerator.SetAmplitude(static_cast<double>(referenceVolume.load(std::memory_order_relaxed)));

            if (outputChannels == 1)
            {
                referenceGenerator.Generate(outputBuffer, true);
            }
            else
            {
                std::span<float> scratchSpan(outputScratchBuffer.data(), frames);
                referenceGenerator.Generate(scratchSpan, false);

                for (size_t i = 0; i < frames; ++i)
                {
                    float sample = outputScratchBuffer[i];
                    outputBuffer[i * 2] += sample;
                    outputBuffer[i * 2 + 1] += sample;
                }
            }
        }
        // Mix polyphonic mode (chord playback) - takes priority over single reference
        else if (polyphonicEnabled.load(std::memory_order_relaxed))
        {
            polyphonicGenerator.SetGlobalVolume(referenceVolume.load(std::memory_order_relaxed));

            if (outputChannels == 1)
            {
                polyphonicGenerator.Generate(outputBuffer, true);
            }
            else
            {
                std::span<float> scratchSpan(outputScratchBuffer.data(), frames);
                polyphonicGenerator.Generate(scratchSpan, false);

                for (size_t i = 0; i < frames; ++i)
                {
                    float sample = outputScratchBuffer[i];
                    outputBuffer[i * 2] += sample;
                    outputBuffer[i * 2 + 1] += sample;
                }
            }
        }
        // Mix reference tone (normal single-shot mode)
        else if (referenceEnabled.load(std::memory_order_relaxed))
        {
            referenceGenerator.SetAmplitude(static_cast<double>(referenceVolume.load(std::memory_order_relaxed)));

            if (outputChannels == 1)
            {
                referenceGenerator.Generate(outputBuffer, true);
            }
            else
            {
                // Generate mono to scratch buffer
                std::span<float> scratchSpan(outputScratchBuffer.data(), frames);
                referenceGenerator.Generate(scratchSpan, false); // Overwrite scratch

                // Mix to stereo output
                for (size_t i = 0; i < frames; ++i)
                {
                    float sample = outputScratchBuffer[i];
                    outputBuffer[i * 2] += sample;
                    outputBuffer[i * 2 + 1] += sample;
                }
            }
        }

        // Mix beep tone (TODO: trigger based on in-tune detection)
        if (beepEnabled.load(std::memory_order_relaxed))
        {
            // beepGenerator.SetAmplitude(static_cast<double>(beepVolume.load(std::memory_order_relaxed)));

            // if (outputChannels == 1)
            // {
            //     beepGenerator.Generate(outputBuffer, true);
            // }
            // else
            // {
            //     std::span<float> scratchSpan(outputScratchBuffer.data(), frames);
            //     beepGenerator.Generate(scratchSpan, false);
            //     for (size_t i = 0; i < frames; ++i)
            //     {
            //         float sample = outputScratchBuffer[i];
            //         outputBuffer[i * 2] += sample;
            //         outputBuffer[i * 2 + 1] += sample;
            //     }
            // }
        }

        // Apply limiting to prevent clipping
        GuitarIO::AudioMixer::Limit(outputBuffer);
    }

    bool AudioProcessingLayer::SwitchInputDevice(uint32_t deviceId)
    {
        LOG_INFO("Switching to input device ID: {}", deviceId);

        if (deviceId == currentInputDeviceId && inputDevice->IsRunning())
        {
            LOG_INFO("Input device {} is already active", deviceId);
            return true;
        }

        if (inputDevice->IsRunning())
        {
            LOG_INFO("Stopping current input stream...");
            if (!inputDevice->Stop())
            {
                LOG_ERROR("Failed to stop input stream: {}", inputDevice->GetLastError());
                return false;
            }
        }

        if (inputDevice->IsOpen())
        {
            LOG_INFO("Closing current input device...");
            inputDevice->Close();
        }

        GuitarIO::AudioStreamConfig inputConfig{
            .sampleRate = config.sampleRate, .bufferSize = config.bufferSize, .inputChannels = 1, .outputChannels = 0
        };

        LOG_INFO("Opening new input device...");
        if (!inputDevice->Open(deviceId, inputConfig, InputCallback, this))
        {
            LOG_ERROR("Failed to open input device: {}", inputDevice->GetLastError());

            // Fallback to default
            LOG_WARN("Attempting to reopen default input device...");
            if (inputDevice->OpenDefault(inputConfig, InputCallback, this))
            {
                inputDevice->Start();
                currentInputDeviceId = static_cast<uint32_t>(-1);
                LOG_INFO("Fallback to default input device successful");
            }
            return false;
        }

        LOG_INFO("Starting new input stream...");
        if (!inputDevice->Start())
        {
            LOG_ERROR("Failed to start input stream: {}", inputDevice->GetLastError());
            inputDevice->Close();

            // Fallback to default
            LOG_WARN("Attempting to reopen default input device...");
            if (inputDevice->OpenDefault(inputConfig, InputCallback, this))
            {
                inputDevice->Start();
                currentInputDeviceId = static_cast<uint32_t>(-1);
                LOG_INFO("Fallback to default input device successful");
            }
            return false;
        }

        currentInputDeviceId = deviceId;

        auto &manager = GuitarIO::AudioDeviceManager::Get();
        auto deviceInfo = manager.GetDeviceInfo(deviceId);
        LOG_INFO("Successfully switched to input device: [{}] {}", deviceId, deviceInfo.name);

        return true;
    }

    bool AudioProcessingLayer::SwitchOutputDevice(uint32_t deviceId)
    {
        LOG_INFO("Switching to output device ID: {}", deviceId);

        if (deviceId == currentOutputDeviceId && outputDevice->IsRunning())
        {
            LOG_INFO("Output device {} is already active", deviceId);
            return true;
        }

        if (outputDevice->IsRunning())
        {
            LOG_INFO("Stopping current output stream...");
            if (!outputDevice->Stop())
            {
                LOG_ERROR("Failed to stop output stream: {}", outputDevice->GetLastError());
                return false;
            }
        }

        if (outputDevice->IsOpen())
        {
            LOG_INFO("Closing current output device...");
            outputDevice->Close();
        }

        auto &manager = GuitarIO::AudioDeviceManager::Get();
        auto deviceInfo = manager.GetDeviceInfo(deviceId);

        // Prefer stereo if available
        uint32_t channels = (deviceInfo.maxOutputChannels >= 2) ? 2 : 1;
        this->outputChannels = channels;

        GuitarIO::AudioStreamConfig outputConfig{ .sampleRate = config.sampleRate,
            .bufferSize = config.bufferSize,
            .inputChannels = 0,
            .outputChannels = channels };

        LOG_INFO("Opening new output device with {} channels...", channels);
        if (!outputDevice->Open(deviceId, outputConfig, OutputCallback, this))
        {
            LOG_ERROR("Failed to open output device: {}", outputDevice->GetLastError());

            // Fallback to default
            LOG_WARN("Attempting to reopen default output device...");
            // Reset to mono for fallback if needed, or query default device info
            // For simplicity, try mono fallback first
            outputConfig.outputChannels = 1;
            this->outputChannels = 1;

            if (outputDevice->OpenDefault(outputConfig, OutputCallback, this))
            {
                outputDevice->Start();
                currentOutputDeviceId = static_cast<uint32_t>(-1);
                LOG_INFO("Fallback to default output device successful (Mono)");
            }
            return false;
        }

        LOG_INFO("Starting new output stream...");
        if (!outputDevice->Start())
        {
            LOG_ERROR("Failed to start output stream: {}", outputDevice->GetLastError());
            outputDevice->Close();

            // Fallback to default
            LOG_WARN("Attempting to reopen default output device...");
            outputConfig.outputChannels = 1;
            this->outputChannels = 1;

            if (outputDevice->OpenDefault(outputConfig, OutputCallback, this))
            {
                outputDevice->Start();
                currentOutputDeviceId = static_cast<uint32_t>(-1);
                LOG_INFO("Fallback to default output device successful (Mono)");
            }
            return false;
        }

        currentOutputDeviceId = deviceId;
        LOG_INFO("Successfully switched to output device: [{}] {}", deviceId, deviceInfo.name);

        return true;
    }

    void AudioProcessingLayer::UpdateAudioFeedback(const AudioConfig &audioConfig)
    {
        beepEnabled.store(audioConfig.enableBeep, std::memory_order_relaxed);
        beepVolume.store(audioConfig.beepVolume, std::memory_order_relaxed);
        referenceEnabled.store(audioConfig.enableReference, std::memory_order_relaxed);
        referenceVolume.store(audioConfig.referenceVolume, std::memory_order_relaxed);
        referenceFrequency.store(audioConfig.referenceFrequency, std::memory_order_relaxed);
        inputMonitoringEnabled.store(audioConfig.enableInputMonitoring, std::memory_order_relaxed);
        monitoringVolume.store(audioConfig.monitoringVolume, std::memory_order_relaxed);
        inputGain.store(audioConfig.inputGain, std::memory_order_relaxed);

        // Advanced modes
        droneEnabled.store(audioConfig.enableDroneMode, std::memory_order_relaxed);
        polyphonicEnabled.store(audioConfig.enablePolyphonicMode, std::memory_order_relaxed);

        // Update generator frequencies
        beepGenerator.SetFrequency(880.0); // A5 for beep
        referenceGenerator.SetFrequency(static_cast<double>(audioConfig.referenceFrequency));

        // Note: Polyphonic frequencies are set by SetPolyphonicFrequencies() called from SettingsLayer
    }

    float AudioProcessingLayer::GetInputLevel() const
    {
        return currentInputLevel.load(std::memory_order_relaxed);
    }

    void AudioProcessingLayer::SetPolyphonicFrequencies(const std::array<float, 6> &frequencies)
    {
        polyphonicGenerator.SetVoiceFrequencies(frequencies);
        polyphonicGenerator.SetGlobalVolume(referenceVolume.load(std::memory_order_relaxed));
    }

} // namespace PrecisionTuner::Layers
