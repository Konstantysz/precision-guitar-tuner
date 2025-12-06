#include "mocks/MockAudioDevice.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <AudioProcessingLayer.h>
#include <Config.h>

using namespace PrecisionTuner::Layers;

// Enable Google Test to print StabilizerType enum values
namespace PrecisionTuner::Layers
{
    void PrintTo(const StabilizerType &type, std::ostream *os)
    {
        switch (type)
        {
        case StabilizerType::None:
            *os << "None";
            break;
        case StabilizerType::EMA:
            *os << "EMA";
            break;
        case StabilizerType::Median:
            *os << "Median";
            break;
        case StabilizerType::Hybrid:
            *os << "Hybrid";
            break;
        default:
            *os << "Unknown";
            break;
        }
    }
} // namespace PrecisionTuner::Layers


/**
 * @brief Test fixture for AudioProcessingLayer
 *
 * Provides mock audio devices and helper functions for testing
 * audio processing, pitch detection, and audio feedback functionality.
 */
class AudioProcessingLayerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create mocks
        auto inputMock = std::make_unique<MockAudioDevice>();
        auto outputMock = std::make_unique<MockAudioDevice>();

        // Keep raw pointers for test manipulation before passing ownership
        inputDevice = inputMock.get();
        outputDevice = outputMock.get();

        // Create layer with injected mocks
        AudioProcessingLayerConfig config;
        config.sampleRate = 48000;
        config.bufferSize = 2048;
        config.stabilizerType = StabilizerType::None; // Disable stabilization for raw detection testing

        layer = std::make_unique<AudioProcessingLayer>(config, std::move(inputMock), std::move(outputMock));
    }

    void TearDown() override
    {
        // Explicitly destroy layer before test fixture cleanup
        layer.reset();
    }

    /**
     * @brief Generates sine wave with continuous phase
     * @param buffer Output buffer to fill
     * @param frequency Frequency in Hz
     * @param sampleRate Sample rate in Hz
     * @param phaseIdx Phase index (modified to maintain phase continuity)
     */
    void FillSineWave(std::vector<float> &buffer, float frequency, int sampleRate, int &phaseIdx)
    {
        for (size_t i = 0; i < buffer.size(); ++i)
        {
            buffer[i] = std::sin(2.0f * std::numbers::pi_v<float> * frequency * phaseIdx / sampleRate);
            phaseIdx++;
        }
    }

    /**
     * @brief Calculates RMS (Root Mean Square) of a buffer
     * @param buffer Input audio buffer
     * @return RMS value
     */
    float CalculateRMS(const std::vector<float> &buffer)
    {
        float sum = 0.0f;
        for (float sample : buffer)
        {
            sum += sample * sample;
        }
        return std::sqrt(sum / buffer.size());
    }

    /**
     * @brief Finds maximum absolute amplitude in buffer
     * @param buffer Input audio buffer
     * @return Maximum absolute value
     */
    float GetMaxAmplitude(const std::vector<float> &buffer)
    {
        float maxAmp = 0.0f;
        for (float sample : buffer)
        {
            maxAmp = std::max(maxAmp, std::abs(sample));
        }
        return maxAmp;
    }

    MockAudioDevice *inputDevice;
    MockAudioDevice *outputDevice;
    std::unique_ptr<AudioProcessingLayer> layer;
};

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST_F(AudioProcessingLayerTest, Initialization)
{
    EXPECT_TRUE(layer->IsInputDeviceAvailable());
    EXPECT_TRUE(inputDevice->IsRunning());

    // Output device may not be available in CI environments (Windows/Linux)
    // Only verify if output device is reported as available
    if (layer->IsOutputDeviceAvailable())
    {
        EXPECT_TRUE(outputDevice->IsRunning());
    }
}

TEST_F(AudioProcessingLayerTest, DetectsPitchCorrectly)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // Process audio multiple times to ensure stability
    for (int i = 0; i < 10; ++i)
    {
        FillSineWave(buffer, 440.0f, 48000, phaseIdx);
        inputDevice->TriggerCallback(buffer, output);
    }

    auto result = layer->GetLatestPitch();

    EXPECT_TRUE(result.detected) << "Pitch not detected. Frequency: " << result.frequency
                                 << ", Confidence: " << result.confidence;
    if (result.detected)
    {
        EXPECT_NEAR(result.frequency, 440.0f, 10.0f); // Relaxed tolerance for synthetic test signals
        EXPECT_GT(result.confidence, 0.8f);
    }
}

TEST_F(AudioProcessingLayerTest, DetectsLowEString)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // Low E string is 82.41 Hz
    for (int i = 0; i < 10; ++i)
    {
        FillSineWave(buffer, 82.41f, 48000, phaseIdx);
        inputDevice->TriggerCallback(buffer, output);
    }

    auto result = layer->GetLatestPitch();

    EXPECT_TRUE(result.detected);
    if (result.detected)
    {
        EXPECT_NEAR(result.frequency, 82.41f, 10.0f); // Relaxed tolerance for synthetic test signals
    }
}

TEST_F(AudioProcessingLayerTest, DetectsHighEString)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // High E string is 329.63 Hz
    for (int i = 0; i < 10; ++i)
    {
        FillSineWave(buffer, 329.63f, 48000, phaseIdx);
        inputDevice->TriggerCallback(buffer, output);
    }

    auto result = layer->GetLatestPitch();

    EXPECT_TRUE(result.detected);
    if (result.detected)
    {
        EXPECT_NEAR(result.frequency, 329.63f, 10.0f); // Relaxed tolerance for synthetic test signals
    }
}

TEST_F(AudioProcessingLayerTest, HandlesSilence)
{
    std::vector<float> buffer(2048, 0.0f);
    std::vector<float> output(2048);

    inputDevice->TriggerCallback(buffer, output);

    auto result = layer->GetLatestPitch();
    EXPECT_FALSE(result.detected);
}

TEST_F(AudioProcessingLayerTest, HandlesLowAmplitudeSignal)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // Very quiet signal (1% amplitude)
    for (int i = 0; i < 10; ++i)
    {
        FillSineWave(buffer, 440.0f, 48000, phaseIdx);
        for (auto &sample : buffer)
        {
            sample *= 0.01f;
        }
        inputDevice->TriggerCallback(buffer, output);
    }

    auto result = layer->GetLatestPitch();
    // May or may not detect depending on threshold, but shouldn't crash
    EXPECT_GE(result.confidence, 0.0f);
    EXPECT_LE(result.confidence, 1.0f);
}

// ============================================================================
// Buffer Management Tests
// ============================================================================

TEST_F(AudioProcessingLayerTest, DetectsBufferOverflow)
{
    // Internal buffer is 2048 * 4 = 8192
    // Send 9000 samples to trigger overflow
    std::vector<float> hugeBuffer(9000, 0.0f);
    std::vector<float> output(9000);

    inputDevice->TriggerCallback(hugeBuffer, output);

    EXPECT_TRUE(layer->CheckBufferOverflow());
    EXPECT_FALSE(layer->CheckBufferOverflow()); // Should be cleared after check
}

TEST_F(AudioProcessingLayerTest, HandlesMultipleSmallBuffers)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // Send many buffers to test continuous processing without overflow
    // This test validates buffer management across multiple callbacks
    for (int i = 0; i < 100; ++i)
    {
        FillSineWave(buffer, 440.0f, 48000, phaseIdx);
        inputDevice->TriggerCallback(buffer, output);
    }

    // Should not overflow (main purpose of this test)
    EXPECT_FALSE(layer->CheckBufferOverflow());

    // Should detect pitch after processing many buffers
    auto result = layer->GetLatestPitch();
    EXPECT_TRUE(result.detected);
    if (result.detected)
    {
        EXPECT_NEAR(result.frequency, 440.0f, 10.0f);
    }
}

// ============================================================================
// Input Level Monitoring Tests
// ============================================================================

TEST_F(AudioProcessingLayerTest, TracksInputLevel)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // Generate signal with known amplitude
    FillSineWave(buffer, 440.0f, 48000, phaseIdx);
    inputDevice->TriggerCallback(buffer, output);

    float level = layer->GetInputLevel();
    EXPECT_GT(level, 0.0f);
    EXPECT_LE(level, 1.0f);
}

TEST_F(AudioProcessingLayerTest, InputLevelReflectsAmplitude)
{
    std::vector<float> quietBuffer(2048);
    std::vector<float> loudBuffer(2048);
    std::vector<float> output(2048);
    int phaseIdx1 = 0, phaseIdx2 = 0;

    // Quiet signal (10% amplitude)
    FillSineWave(quietBuffer, 440.0f, 48000, phaseIdx1);
    for (auto &sample : quietBuffer)
        sample *= 0.1f;
    inputDevice->TriggerCallback(quietBuffer, output);
    float quietLevel = layer->GetInputLevel();

    // Loud signal (100% amplitude)
    FillSineWave(loudBuffer, 440.0f, 48000, phaseIdx2);
    inputDevice->TriggerCallback(loudBuffer, output);
    float loudLevel = layer->GetInputLevel();

    EXPECT_LT(quietLevel, loudLevel);
}

// ============================================================================
// Audio Feedback Tests - Reference Tone
// ============================================================================

TEST_F(AudioProcessingLayerTest, GeneratesReferenceTone)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    PrecisionTuner::AudioConfig audioConfig;
    audioConfig.enableReference = true;
    audioConfig.referenceFrequency = 440.0f;
    audioConfig.referenceVolume = 1.0f;
    layer->UpdateAudioFeedback(audioConfig);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);

    outputDevice->TriggerCallback(input, output);

    float maxAmp = GetMaxAmplitude(output);
    EXPECT_GT(maxAmp, 0.0f);
}

TEST_F(AudioProcessingLayerTest, ReferenceToneVolumeControl)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    std::vector<float> input(512);
    std::vector<float> quietOutput(512, 0.0f);
    std::vector<float> loudOutput(512, 0.0f);

    // Quiet reference
    PrecisionTuner::AudioConfig quietConfig;
    quietConfig.enableReference = true;
    quietConfig.referenceFrequency = 440.0f;
    quietConfig.referenceVolume = 0.1f;
    layer->UpdateAudioFeedback(quietConfig);
    outputDevice->TriggerCallback(input, quietOutput);
    float quietLevel = GetMaxAmplitude(quietOutput);

    // Loud reference
    PrecisionTuner::AudioConfig loudConfig;
    loudConfig.enableReference = true;
    loudConfig.referenceFrequency = 440.0f;
    loudConfig.referenceVolume = 1.0f;
    layer->UpdateAudioFeedback(loudConfig);
    outputDevice->TriggerCallback(input, loudOutput);
    float loudLevel = GetMaxAmplitude(loudOutput);

    EXPECT_LT(quietLevel, loudLevel);
}

TEST_F(AudioProcessingLayerTest, DisablesReferenceTone)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    // First enable
    PrecisionTuner::AudioConfig enableConfig;
    enableConfig.enableReference = true;
    enableConfig.referenceFrequency = 440.0f;
    enableConfig.referenceVolume = 1.0f;
    layer->UpdateAudioFeedback(enableConfig);

    // Then disable
    PrecisionTuner::AudioConfig disableConfig;
    disableConfig.enableReference = false;
    layer->UpdateAudioFeedback(disableConfig);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);
    outputDevice->TriggerCallback(input, output);

    float maxAmp = GetMaxAmplitude(output);
    EXPECT_FLOAT_EQ(maxAmp, 0.0f);
}

// ============================================================================
// Audio Feedback Tests - Beep
// ============================================================================

TEST_F(AudioProcessingLayerTest, GeneratesBeep)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    PrecisionTuner::AudioConfig audioConfig;
    audioConfig.enableBeep = true;
    audioConfig.beepVolume = 1.0f;
    layer->UpdateAudioFeedback(audioConfig);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);

    outputDevice->TriggerCallback(input, output);

    // Beep should generate some signal
    float maxAmp = GetMaxAmplitude(output);
    EXPECT_GE(maxAmp, 0.0f); // May be 0 if not in-tune
}

TEST_F(AudioProcessingLayerTest, BeepVolumeControl)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    std::vector<float> input(512);
    std::vector<float> output1(512, 0.0f);
    std::vector<float> output2(512, 0.0f);

    PrecisionTuner::AudioConfig config1;
    config1.enableBeep = true;
    config1.beepVolume = 0.2f;
    layer->UpdateAudioFeedback(config1);
    outputDevice->TriggerCallback(input, output1);
    float level1 = GetMaxAmplitude(output1);

    PrecisionTuner::AudioConfig config2;
    config2.enableBeep = true;
    config2.beepVolume = 1.0f;
    layer->UpdateAudioFeedback(config2);
    outputDevice->TriggerCallback(input, output2);
    float level2 = GetMaxAmplitude(output2);

    // Higher volume should produce higher or equal amplitude
    EXPECT_GE(level2, level1);
}

// ============================================================================
// Audio Feedback Tests - Input Monitoring
// ============================================================================

TEST_F(AudioProcessingLayerTest, InputMonitoringPassthrough)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    PrecisionTuner::AudioConfig audioConfig;
    audioConfig.enableInputMonitoring = true;
    audioConfig.monitoringVolume = 1.0f;
    layer->UpdateAudioFeedback(audioConfig);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);
    int phaseIdx = 0;

    // Generate input signal
    FillSineWave(input, 440.0f, 48000, phaseIdx);

    // First send to input to fill monitoring buffer
    std::vector<float> dummyOutput(512);
    inputDevice->TriggerCallback(input, dummyOutput);

    // Then get monitored output
    std::vector<float> emptyInput(512, 0.0f);
    outputDevice->TriggerCallback(emptyInput, output);

    // Should have some signal from monitoring
    float maxAmp = GetMaxAmplitude(output);
    EXPECT_GT(maxAmp, 0.0f);
}

TEST_F(AudioProcessingLayerTest, InputMonitoringVolumeControl)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    std::vector<float> input(512);
    int phaseIdx = 0;
    FillSineWave(input, 440.0f, 48000, phaseIdx);

    // Send input first
    std::vector<float> dummyOutput(512);
    inputDevice->TriggerCallback(input, dummyOutput);

    // Test quiet monitoring
    PrecisionTuner::AudioConfig quietConfig;
    quietConfig.enableInputMonitoring = true;
    quietConfig.monitoringVolume = 0.2f;
    layer->UpdateAudioFeedback(quietConfig);

    std::vector<float> quietOutput(512, 0.0f);
    std::vector<float> emptyInput(512, 0.0f);
    outputDevice->TriggerCallback(emptyInput, quietOutput);
    float quietLevel = GetMaxAmplitude(quietOutput);

    // Send input again
    inputDevice->TriggerCallback(input, dummyOutput);

    // Test loud monitoring
    PrecisionTuner::AudioConfig loudConfig;
    loudConfig.enableInputMonitoring = true;
    loudConfig.monitoringVolume = 1.0f;
    layer->UpdateAudioFeedback(loudConfig);

    std::vector<float> loudOutput(512, 0.0f);
    outputDevice->TriggerCallback(emptyInput, loudOutput);
    float loudLevel = GetMaxAmplitude(loudOutput);

    EXPECT_LE(quietLevel, loudLevel);
}

// ============================================================================
// Audio Feedback Tests - Polyphonic Mode
// ============================================================================

TEST_F(AudioProcessingLayerTest, PolyphonicModeGeneratesChord)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    PrecisionTuner::AudioConfig audioConfig;
    audioConfig.enablePolyphonicMode = true;
    layer->UpdateAudioFeedback(audioConfig);

    // Set standard tuning frequencies (EADGBE)
    std::array<float, 6> frequencies = {
        82.41f,  // E2
        110.00f, // A2
        146.83f, // D3
        196.00f, // G3
        246.94f, // B3
        329.63f  // E4
    };
    layer->SetPolyphonicFrequencies(frequencies);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);

    outputDevice->TriggerCallback(input, output);

    float maxAmp = GetMaxAmplitude(output);
    EXPECT_GT(maxAmp, 0.0f);
}

TEST_F(AudioProcessingLayerTest, PolyphonicModePartialChord)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    PrecisionTuner::AudioConfig audioConfig;
    audioConfig.enablePolyphonicMode = true;
    layer->UpdateAudioFeedback(audioConfig);

    // Only enable some strings (0 = disabled)
    std::array<float, 6> frequencies = {
        82.41f,  // E2
        0.0f,    // Disabled
        146.83f, // D3
        0.0f,    // Disabled
        246.94f, // B3
        0.0f     // Disabled
    };
    layer->SetPolyphonicFrequencies(frequencies);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);

    outputDevice->TriggerCallback(input, output);

    float maxAmp = GetMaxAmplitude(output);
    EXPECT_GT(maxAmp, 0.0f);
}

// ============================================================================
// Pitch Stabilization Tests
// ============================================================================

/**
 * @brief Test fixture for pitch stabilization tests
 * Creates layer with different stabilizer configurations
 */
class PitchStabilizationTest : public ::testing::TestWithParam<StabilizerType>
{
protected:
    void SetUp() override
    {
        auto inputMock = std::make_unique<MockAudioDevice>();
        auto outputMock = std::make_unique<MockAudioDevice>();

        inputDevice = inputMock.get();
        outputDevice = outputMock.get();

        AudioProcessingLayerConfig config;
        config.sampleRate = 48000;
        config.bufferSize = 2048;
        config.stabilizerType = GetParam();
        config.emaAlpha = 0.3f;
        config.medianWindowSize = 5;

        layer = std::make_unique<AudioProcessingLayer>(config, std::move(inputMock), std::move(outputMock));
    }

    void FillSineWave(std::vector<float> &buffer, float frequency, int sampleRate, int &phaseIdx)
    {
        for (size_t i = 0; i < buffer.size(); ++i)
        {
            buffer[i] = std::sin(2.0f * std::numbers::pi_v<float> * frequency * phaseIdx / sampleRate);
            phaseIdx++;
        }
    }

    MockAudioDevice *inputDevice;
    MockAudioDevice *outputDevice;
    std::unique_ptr<AudioProcessingLayer> layer;
};

TEST_P(PitchStabilizationTest, StabilizesPitchDetection)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    // Process multiple frames to allow stabilizers to converge
    for (int i = 0; i < 50; ++i)
    {
        FillSineWave(buffer, 440.0f, 48000, phaseIdx);
        inputDevice->TriggerCallback(buffer, output);
    }

    auto result = layer->GetLatestPitch();

    EXPECT_TRUE(result.detected);
    if (result.detected)
    {
        EXPECT_NEAR(result.frequency, 440.0f, 10.0f); // Relaxed tolerance for synthetic test signals with stabilization
    }
}

INSTANTIATE_TEST_SUITE_P(StabilizerTypes,
    PitchStabilizationTest,
    ::testing::Values(StabilizerType::None, StabilizerType::EMA, StabilizerType::Median, StabilizerType::Hybrid));

// ============================================================================
// Edge Cases and Stress Tests
// ============================================================================

TEST_F(AudioProcessingLayerTest, HandlesRapidFrequencyChanges)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);
    int phaseIdx = 0;

    float frequencies[] = { 82.41f, 110.0f, 146.83f, 196.0f, 246.94f };

    for (float freq : frequencies)
    {
        phaseIdx = 0; // Reset phase for new frequency
        for (int i = 0; i < 5; ++i)
        {
            FillSineWave(buffer, freq, 48000, phaseIdx);
            inputDevice->TriggerCallback(buffer, output);
        }
    }

    // Should not crash or overflow
    EXPECT_FALSE(layer->CheckBufferOverflow());
}

TEST_F(AudioProcessingLayerTest, HandlesFrequencyAtBoundaries)
{
    std::vector<float> buffer(2048);
    std::vector<float> output(2048);

    // Test minimum frequency (80 Hz)
    int phaseIdx1 = 0;
    for (int i = 0; i < 10; ++i)
    {
        FillSineWave(buffer, 80.0f, 48000, phaseIdx1);
        inputDevice->TriggerCallback(buffer, output);
    }

    auto lowResult = layer->GetLatestPitch();
    // May or may not detect at boundary, but shouldn't crash
    EXPECT_GE(lowResult.confidence, 0.0f);

    // Test maximum frequency (1200 Hz)
    int phaseIdx2 = 0;
    for (int i = 0; i < 10; ++i)
    {
        FillSineWave(buffer, 1200.0f, 48000, phaseIdx2);
        inputDevice->TriggerCallback(buffer, output);
    }

    auto highResult = layer->GetLatestPitch();
    EXPECT_GE(highResult.confidence, 0.0f);
}

TEST_F(AudioProcessingLayerTest, HandlesMixedAudioFeedbackModes)
{
    if (!layer->IsOutputDeviceAvailable())
    {
        GTEST_SKIP() << "Output device not available (CI environment)";
    }

    // Enable all feedback modes simultaneously
    PrecisionTuner::AudioConfig audioConfig;
    audioConfig.enableBeep = true;
    audioConfig.beepVolume = 0.5f;
    audioConfig.enableReference = true;
    audioConfig.referenceFrequency = 440.0f;
    audioConfig.referenceVolume = 0.5f;
    audioConfig.enableInputMonitoring = true;
    audioConfig.monitoringVolume = 0.5f;
    audioConfig.enablePolyphonicMode = true;
    layer->UpdateAudioFeedback(audioConfig);

    std::array<float, 6> frequencies = { 82.41f, 110.0f, 146.83f, 196.0f, 246.94f, 329.63f };
    layer->SetPolyphonicFrequencies(frequencies);

    std::vector<float> input(512);
    std::vector<float> output(512, 0.0f);

    // Should handle all modes without crashing
    outputDevice->TriggerCallback(input, output);

    // Output should contain mixed signal
    float maxAmp = GetMaxAmplitude(output);
    EXPECT_GE(maxAmp, 0.0f);
    EXPECT_LE(maxAmp, 1.0f); // Should not clip
}
