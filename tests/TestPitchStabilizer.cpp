#include <gtest/gtest.h>
#include <PitchStabilizer.h>

using namespace GuitarDSP;

// ========== Helper Functions ==========

PitchResult MakePitch(float frequency, float confidence = 0.9f)
{
    return PitchResult{ frequency, confidence };
}

// ========== ExponentialMovingAverage Tests ==========

TEST(ExponentialMovingAverage, InitialValueIsFirstSample)
{
    ExponentialMovingAverage ema(ExponentialMovingAverageConfig{ .alpha = 0.3f });

    PitchResult input = MakePitch(440.0f, 0.9f);
    ema.Update(input);

    PitchResult result = ema.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 440.0f);
    EXPECT_FLOAT_EQ(result.confidence, 0.9f);
}

TEST(ExponentialMovingAverage, ConvergesToStableValue)
{
    ExponentialMovingAverage ema(ExponentialMovingAverageConfig{ .alpha = 0.3f });

    // Feed constant value multiple times
    for (int i = 0; i < 20; ++i)
    {
        ema.Update(MakePitch(440.0f, 0.9f));
    }

    PitchResult result = ema.GetStabilized();
    EXPECT_NEAR(result.frequency, 440.0f, 0.01f);
    EXPECT_NEAR(result.confidence, 0.9f, 0.01f);
}

TEST(ExponentialMovingAverage, RespondsToStepChange)
{
    ExponentialMovingAverage ema(ExponentialMovingAverageConfig{ .alpha = 0.5f }); // Higher alpha for faster response

    // Initialize with one value
    ema.Update(MakePitch(440.0f, 0.9f));

    // Step change to new value
    ema.Update(MakePitch(880.0f, 0.8f));

    PitchResult result = ema.GetStabilized();

    // Should be between old and new value (weighted average)
    EXPECT_GT(result.frequency, 440.0f);
    EXPECT_LT(result.frequency, 880.0f);
}

TEST(ExponentialMovingAverage, AlphaAffectsConvergenceRate)
{
    ExponentialMovingAverage fastEMA(ExponentialMovingAverageConfig{ .alpha = 0.9f }); // Fast
    ExponentialMovingAverage slowEMA(ExponentialMovingAverageConfig{ .alpha = 0.1f }); // Slow

    // Initialize both
    fastEMA.Update(MakePitch(440.0f, 0.9f));
    slowEMA.Update(MakePitch(440.0f, 0.9f));

    // Apply step change
    fastEMA.Update(MakePitch(880.0f, 0.9f));
    slowEMA.Update(MakePitch(880.0f, 0.9f));

    PitchResult fastResult = fastEMA.GetStabilized();
    PitchResult slowResult = slowEMA.GetStabilized();

    // Fast EMA should be closer to new value
    EXPECT_GT(fastResult.frequency, slowResult.frequency);
}

TEST(ExponentialMovingAverage, ResetClearsState)
{
    ExponentialMovingAverage ema(ExponentialMovingAverageConfig{ .alpha = 0.3f });

    ema.Update(MakePitch(440.0f, 0.9f));
    ema.Reset();

    PitchResult result = ema.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 0.0f);
    EXPECT_FLOAT_EQ(result.confidence, 0.0f);
}

// ========== MedianFilter Tests ==========

TEST(MedianFilter, InitialValueIsFirstSample)
{
    MedianFilter filter(MedianFilterConfig{ .windowSize = 5 });

    PitchResult input = MakePitch(440.0f, 0.9f);
    filter.Update(input);

    PitchResult result = filter.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 440.0f);
    EXPECT_FLOAT_EQ(result.confidence, 0.9f);
}

TEST(MedianFilter, RejectsSingleSpike)
{
    MedianFilter filter(MedianFilterConfig{ .windowSize = 5 });

    // Feed normal values
    filter.Update(MakePitch(440.0f, 0.9f));
    filter.Update(MakePitch(441.0f, 0.9f));
    filter.Update(MakePitch(439.0f, 0.9f));

    // Insert spike
    filter.Update(MakePitch(2000.0f, 0.5f)); // Anomaly

    // More normal values
    filter.Update(MakePitch(440.5f, 0.9f));

    PitchResult result = filter.GetStabilized();

    // Median should ignore the spike
    EXPECT_NEAR(result.frequency, 440.0f, 2.0f); // Within reasonable range
}

TEST(MedianFilter, HandlesOddWindowSize)
{
    MedianFilter filter(MedianFilterConfig{ .windowSize = 5 });

    // Fill window: [100, 200, 300, 400, 500]
    filter.Update(MakePitch(100.0f));
    filter.Update(MakePitch(200.0f));
    filter.Update(MakePitch(300.0f));
    filter.Update(MakePitch(400.0f));
    filter.Update(MakePitch(500.0f));

    PitchResult result = filter.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 300.0f); // Middle value
}

TEST(MedianFilter, HandlesEvenWindowSize)
{
    MedianFilter filter(MedianFilterConfig{ .windowSize = 4 });

    // Fill window: [100, 200, 300, 400]
    filter.Update(MakePitch(100.0f));
    filter.Update(MakePitch(200.0f));
    filter.Update(MakePitch(300.0f));
    filter.Update(MakePitch(400.0f));

    PitchResult result = filter.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 250.0f); // Average of two middle values (200 + 300) / 2
}

TEST(MedianFilter, ResetClearsWindow)
{
    MedianFilter filter(MedianFilterConfig{ .windowSize = 5 });

    filter.Update(MakePitch(440.0f, 0.9f));
    filter.Update(MakePitch(441.0f, 0.9f));
    filter.Reset();

    filter.Update(MakePitch(220.0f, 0.8f));

    PitchResult result = filter.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 220.0f); // Only the new value after reset
}

// ========== HybridStabilizer Tests ==========

TEST(HybridStabilizer, InitialValueIsFirstSample)
{
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.3f, .windowSize = 5 });

    PitchResult input = MakePitch(440.0f, 0.9f);
    hybrid.Update(input);

    PitchResult result = hybrid.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 440.0f);
}

TEST(HybridStabilizer, HighConfidenceFasterConvergence)
{
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.3f, .windowSize = 3 });

    // Initialize
    hybrid.Update(MakePitch(440.0f, 0.9f));

    // High confidence change
    hybrid.Update(MakePitch(880.0f, 0.95f));

    PitchResult highConfResult = hybrid.GetStabilized();

    // Reset and try with low confidence
    hybrid.Reset();
    hybrid.Update(MakePitch(440.0f, 0.9f));
    hybrid.Update(MakePitch(880.0f, 0.2f)); // Low confidence

    PitchResult lowConfResult = hybrid.GetStabilized();

    // High confidence should converge faster (be closer to 880)
    EXPECT_GT(highConfResult.frequency, lowConfResult.frequency);
}

TEST(HybridStabilizer, RejectsSpikesLikeMedianFilter)
{
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.3f, .windowSize = 5 });

    // Normal values
    hybrid.Update(MakePitch(440.0f, 0.9f));
    hybrid.Update(MakePitch(441.0f, 0.9f));
    hybrid.Update(MakePitch(439.0f, 0.9f));

    // Spike
    hybrid.Update(MakePitch(2000.0f, 0.3f));

    // Normal value
    hybrid.Update(MakePitch(440.5f, 0.9f));

    PitchResult result = hybrid.GetStabilized();

    // Should reject spike like median filter
    EXPECT_NEAR(result.frequency, 440.0f, 50.0f);
}

TEST(HybridStabilizer, CombinesMedianAndEMA)
{
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.5f, .windowSize = 3 });

    // Feed gradual change
    hybrid.Update(MakePitch(440.0f, 0.9f));
    hybrid.Update(MakePitch(445.0f, 0.9f));
    hybrid.Update(MakePitch(450.0f, 0.9f));

    PitchResult result = hybrid.GetStabilized();

    // Should smooth the progression (not just return median)
    // Value should be influenced by both median and EMA
    EXPECT_GT(result.frequency, 440.0f);
    EXPECT_LT(result.frequency, 450.0f);
}

TEST(HybridStabilizer, ResetClearsAllState)
{
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.3f, .windowSize = 5 });

    hybrid.Update(MakePitch(440.0f, 0.9f));
    hybrid.Update(MakePitch(441.0f, 0.9f));
    hybrid.Reset();

    hybrid.Update(MakePitch(220.0f, 0.8f));

    PitchResult result = hybrid.GetStabilized();
    EXPECT_FLOAT_EQ(result.frequency, 220.0f);
}

// ========== Real-world Scenario Tests ==========

TEST(PitchStabilizer, JitteryInputSmoothing)
{
    // Simulate jittery guitar input (±2 Hz oscillation around 82.41 Hz - low E string)
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.3f, .windowSize = 5 });

    float baseFreq = 82.41f;
    float jitter[] = { 0.0f, 1.5f, -1.2f, 0.8f, -1.8f, 1.0f, -0.5f, 1.3f, -1.4f, 0.6f };

    for (float offset : jitter)
    {
        hybrid.Update(MakePitch(baseFreq + offset, 0.85f));
    }

    PitchResult result = hybrid.GetStabilized();

    // Should be close to base frequency despite jitter
    EXPECT_NEAR(result.frequency, baseFreq, 1.0f);
}

TEST(PitchStabilizer, StepChangeConvergence)
{
    // Test transition from E2 (82.41 Hz) to A2 (110.0 Hz)
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.4f, .windowSize = 5 });

    // Start at E2
    for (int i = 0; i < 10; ++i)
    {
        hybrid.Update(MakePitch(82.41f, 0.9f));
    }

    // Change to A2
    for (int i = 0; i < 15; ++i)
    {
        hybrid.Update(MakePitch(110.0f, 0.9f));
    }

    PitchResult result = hybrid.GetStabilized();

    // Should converge to new value
    EXPECT_NEAR(result.frequency, 110.0f, 5.0f);
}

TEST(PitchStabilizer, TransientSpikeRejection)
{
    // Test rejection of brief anomaly in sustained note
    HybridStabilizer hybrid(HybridStabilizerConfig{ .baseAlpha = 0.3f, .windowSize = 5 });

    // Sustained E2
    for (int i = 0; i < 5; ++i)
    {
        hybrid.Update(MakePitch(82.41f, 0.9f));
    }

    // Brief spike (maybe from picking adjacent string)
    hybrid.Update(MakePitch(196.0f, 0.4f));

    // Back to sustained E2
    for (int i = 0; i < 5; ++i)
    {
        hybrid.Update(MakePitch(82.41f, 0.9f));
    }

    PitchResult result = hybrid.GetStabilized();

    // Should maintain E2, rejecting spike
    EXPECT_NEAR(result.frequency, 82.41f, 5.0f);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
