/**
 * Unit Tests for TuningPresets
 * Tests frequency calculations, string detection, and edge cases
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include <gtest/gtest.h>
#include <TuningPresets.h>
#include <NoteConverter.h>
#include <cmath>
#include <format>

using namespace PrecisionTuner;

// Test tolerance for frequency comparisons (Hz)
constexpr float FREQ_TOLERANCE = 0.01f;

// Test tolerance for cent comparisons
constexpr float CENTS_TOLERANCE = 0.1f;

TEST(TuningPresetsTest, StandardTuning_A440_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 440.0f);
    
    EXPECT_EQ(preset.name, "Standard (EADGBE)");
    
    // Expected frequencies at A440
    EXPECT_NEAR(preset.targetFrequencies[0], 82.41f, FREQ_TOLERANCE);   // E2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f, FREQ_TOLERANCE);  // A2
    EXPECT_NEAR(preset.targetFrequencies[2], 146.83f, FREQ_TOLERANCE);  // D3
    EXPECT_NEAR(preset.targetFrequencies[3], 196.00f, FREQ_TOLERANCE);  // G3
    EXPECT_NEAR(preset.targetFrequencies[4], 246.94f, FREQ_TOLERANCE);  // B3
    EXPECT_NEAR(preset.targetFrequencies[5], 329.63f, FREQ_TOLERANCE);  // E4
    
    // Verify note names
    EXPECT_EQ(preset.noteNames[0], "E2");
    EXPECT_EQ(preset.noteNames[1], "A2");
    EXPECT_EQ(preset.noteNames[2], "D3");
    EXPECT_EQ(preset.noteNames[3], "G3");
    EXPECT_EQ(preset.noteNames[4], "B3");
    EXPECT_EQ(preset.noteNames[5], "E4");
}

TEST(TuningPresetsTest, StandardTuning_A442_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 442.0f);
    
    // At A442, frequencies should scale proportionally
    // A2 = 110Hz at A440, should be 110 * (442/440) = 110.5Hz at A442
    float scaleFactor = 442.0f / 440.0f;
    
    EXPECT_NEAR(preset.targetFrequencies[0], 82.41f * scaleFactor, FREQ_TOLERANCE);   // E2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f * scaleFactor, FREQ_TOLERANCE);  // A2
    EXPECT_NEAR(preset.targetFrequencies[2], 146.83f * scaleFactor, FREQ_TOLERANCE);  // D3
    EXPECT_NEAR(preset.targetFrequencies[3], 196.00f * scaleFactor, FREQ_TOLERANCE);  // G3
    EXPECT_NEAR(preset.targetFrequencies[4], 246.94f * scaleFactor, FREQ_TOLERANCE);  // B3
    EXPECT_NEAR(preset.targetFrequencies[5], 329.63f * scaleFactor, FREQ_TOLERANCE);  // E4
}

TEST(TuningPresetsTest, DropD_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::DropD, 440.0f);
    
    EXPECT_EQ(preset.name, "Drop D");
    
    // Drop D: D-A-D-G-B-E
    // 6th string (E2) drops to D2 (73.42 Hz)
    EXPECT_NEAR(preset.targetFrequencies[0], 73.42f, FREQ_TOLERANCE);   // D2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f, FREQ_TOLERANCE);  // A2
    EXPECT_NEAR(preset.targetFrequencies[2], 146.83f, FREQ_TOLERANCE);  // D3
    EXPECT_NEAR(preset.targetFrequencies[3], 196.00f, FREQ_TOLERANCE);  // G3
    EXPECT_NEAR(preset.targetFrequencies[4], 246.94f, FREQ_TOLERANCE);  // B3
    EXPECT_NEAR(preset.targetFrequencies[5], 329.63f, FREQ_TOLERANCE);  // E4
}

TEST(TuningPresetsTest, DropC_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::DropC, 440.0f);
    
    EXPECT_EQ(preset.name, "Drop C");
    
    // Drop C: C-G-C-F-A-D
    EXPECT_NEAR(preset.targetFrequencies[0], 65.41f, FREQ_TOLERANCE);   // C2
    EXPECT_NEAR(preset.targetFrequencies[1], 98.00f, FREQ_TOLERANCE);   // G2
    EXPECT_NEAR(preset.targetFrequencies[2], 130.81f, FREQ_TOLERANCE);  // C3
    EXPECT_NEAR(preset.targetFrequencies[3], 174.61f, FREQ_TOLERANCE);  // F3
    EXPECT_NEAR(preset.targetFrequencies[4], 220.00f, FREQ_TOLERANCE);  // A3
    EXPECT_NEAR(preset.targetFrequencies[5], 293.66f, FREQ_TOLERANCE);  // D4
}

TEST(TuningPresetsTest, DADGAD_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::DADGAD, 440.0f);
    
    EXPECT_EQ(preset.name, "DADGAD");
    
    // DADGAD: D-A-D-G-A-D
    EXPECT_NEAR(preset.targetFrequencies[0], 73.42f, FREQ_TOLERANCE);   // D2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f, FREQ_TOLERANCE);  // A2
    EXPECT_NEAR(preset.targetFrequencies[2], 146.83f, FREQ_TOLERANCE);  // D3
    EXPECT_NEAR(preset.targetFrequencies[3], 196.00f, FREQ_TOLERANCE);  // G3
    EXPECT_NEAR(preset.targetFrequencies[4], 220.00f, FREQ_TOLERANCE);  // A3
    EXPECT_NEAR(preset.targetFrequencies[5], 293.66f, FREQ_TOLERANCE);  // D4
}

TEST(TuningPresetsTest, OpenG_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::OpenG, 440.0f);
    
    EXPECT_EQ(preset.name, "Open G");
    
    // Open G: D-G-D-G-B-D
    EXPECT_NEAR(preset.targetFrequencies[0], 73.42f, FREQ_TOLERANCE);   // D2
    EXPECT_NEAR(preset.targetFrequencies[1], 98.00f, FREQ_TOLERANCE);   // G2
    EXPECT_NEAR(preset.targetFrequencies[2], 146.83f, FREQ_TOLERANCE);  // D3
    EXPECT_NEAR(preset.targetFrequencies[3], 196.00f, FREQ_TOLERANCE);  // G3
    EXPECT_NEAR(preset.targetFrequencies[4], 246.94f, FREQ_TOLERANCE);  // B3
    EXPECT_NEAR(preset.targetFrequencies[5], 293.66f, FREQ_TOLERANCE);  // D4
}

TEST(TuningPresetsTest, OpenD_CorrectFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::OpenD, 440.0f);
    
    EXPECT_EQ(preset.name, "Open D");
    
    // Open D: D-A-D-F#-A-D
    EXPECT_NEAR(preset.targetFrequencies[0], 73.42f, FREQ_TOLERANCE);   // D2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f, FREQ_TOLERANCE);  // A2
    EXPECT_NEAR(preset.targetFrequencies[2], 146.83f, FREQ_TOLERANCE);  // D3
    EXPECT_NEAR(preset.targetFrequencies[3], 185.00f, FREQ_TOLERANCE);  // F#3
    EXPECT_NEAR(preset.targetFrequencies[4], 220.00f, FREQ_TOLERANCE);  // A3
    EXPECT_NEAR(preset.targetFrequencies[5], 293.66f, FREQ_TOLERANCE);  // D4
}

TEST(TuningPresetsTest, ChromaticMode_NoTargetFrequencies)
{
    auto preset = TuningPresets::GetPreset(TuningMode::Chromatic, 440.0f);
    
    EXPECT_EQ(preset.name, "Chromatic");
    
    // Chromatic mode should have zero frequencies
    for (size_t i = 0; i < 6; ++i)
    {
        EXPECT_EQ(preset.targetFrequencies[i], 0.0f);
        EXPECT_TRUE(preset.noteNames[i].empty());
    }
}

TEST(TuningPresetsTest, FindClosestString_ExactMatch)
{
    // Test exact frequency match for standard tuning
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, 82.41f, 440.0f, 25.0f);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0); // 6th string (E2)
    
    result = TuningPresets::FindClosestString(TuningMode::Standard, 110.00f, 440.0f, 25.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1); // 5th string (A2)
    
    result = TuningPresets::FindClosestString(TuningMode::Standard, 329.63f, 440.0f, 25.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 5); // 1st string (E4)
}

TEST(TuningPresetsTest, FindClosestString_WithinTolerance)
{
    // Test frequency slightly sharp (+10 cents from E2)
    // 10 cents = frequency * 2^(10/1200)
    float sharpE2 = 82.41f * std::pow(2.0f, 10.0f / 1200.0f); // ~82.89 Hz
    
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, sharpE2, 440.0f, 25.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0); // Should still match 6th string
    
    // Test frequency slightly flat (-15 cents from A2)
    float flatA2 = 110.00f * std::pow(2.0f, -15.0f / 1200.0f); // ~109.04 Hz
    
    result = TuningPresets::FindClosestString(TuningMode::Standard, flatA2, 440.0f, 25.0f);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1); // Should still match 5th string
}

TEST(TuningPresetsTest, FindClosestString_OutsideTolerance)
{
    // Test frequency way too sharp (50 cents from E2, outside default 25-cent tolerance)
    float verySharpE2 = 82.41f * std::pow(2.0f, 50.0f / 1200.0f); // ~85.02 Hz
    
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, verySharpE2, 440.0f, 25.0f);
    EXPECT_FALSE(result.has_value()); // Should not match any string
    
    // Test with extremely low frequency
    result = TuningPresets::FindClosestString(TuningMode::Standard, 50.0f, 440.0f, 25.0f);
    EXPECT_FALSE(result.has_value());
    
    // Test with extremely high frequency
    result = TuningPresets::FindClosestString(TuningMode::Standard, 500.0f, 440.0f, 25.0f);
    EXPECT_FALSE(result.has_value());
}

TEST(TuningPresetsTest, FindClosestString_ChromaticMode)
{
    // Chromatic mode should always return nullopt
    auto result = TuningPresets::FindClosestString(TuningMode::Chromatic, 82.41f, 440.0f, 25.0f);
    EXPECT_FALSE(result.has_value());
    
    result = TuningPresets::FindClosestString(TuningMode::Chromatic, 440.0f, 440.0f, 25.0f);
    EXPECT_FALSE(result.has_value());
}

TEST(TuningPresetsTest, GetStringName_StandardTuning)
{
    // Test all string indices with standard tuning
    EXPECT_EQ(TuningPresets::GetStringName(0, TuningMode::Standard), "6th String (E2)");
    EXPECT_EQ(TuningPresets::GetStringName(1, TuningMode::Standard), "5th String (A2)");
    EXPECT_EQ(TuningPresets::GetStringName(2, TuningMode::Standard), "4th String (D3)");
    EXPECT_EQ(TuningPresets::GetStringName(3, TuningMode::Standard), "3rd String (G3)");
    EXPECT_EQ(TuningPresets::GetStringName(4, TuningMode::Standard), "2nd String (B3)");
    EXPECT_EQ(TuningPresets::GetStringName(5, TuningMode::Standard), "1st String (E4)");
}

TEST(TuningPresetsTest, GetStringName_DropD)
{
    // Test Drop D tuning (first string is D2 instead of E2)
    EXPECT_EQ(TuningPresets::GetStringName(0, TuningMode::DropD), "6th String (D2)");
    EXPECT_EQ(TuningPresets::GetStringName(5, TuningMode::DropD), "1st String (E4)");
}

TEST(TuningPresetsTest, GetStringName_InvalidIndices)
{
    // Test invalid indices
    EXPECT_EQ(TuningPresets::GetStringName(-1, TuningMode::Standard), "Unknown String");
    EXPECT_EQ(TuningPresets::GetStringName(6, TuningMode::Standard), "Unknown String");
    EXPECT_EQ(TuningPresets::GetStringName(100, TuningMode::Standard), "Unknown String");
}

TEST(TuningPresetsTest, ReferencePitchAdjustment_A430)
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 430.0f);
    
    // At A430, frequencies should scale proportionally
    float scaleFactor = 430.0f / 440.0f;
    
    EXPECT_NEAR(preset.targetFrequencies[0], 82.41f * scaleFactor, FREQ_TOLERANCE);   // E2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f * scaleFactor, FREQ_TOLERANCE);  // A2
}

TEST(TuningPresetsTest, ReferencePitchAdjustment_A450)
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 450.0f);
    
    // At A450, frequencies should scale proportionally
    float scaleFactor = 450.0f / 440.0f;
    
    EXPECT_NEAR(preset.targetFrequencies[0], 82.41f * scaleFactor, FREQ_TOLERANCE);   // E2
    EXPECT_NEAR(preset.targetFrequencies[1], 110.00f * scaleFactor, FREQ_TOLERANCE);  // A2
}

TEST(TuningPresetsTest, GetAllPresets)
{
    auto presets = TuningPresets::GetAllPresets(440.0f);
    
    ASSERT_EQ(presets.size(), 7);
    EXPECT_EQ(presets[0].name, "Chromatic");
    EXPECT_EQ(presets[1].name, "Standard (EADGBE)");
    EXPECT_EQ(presets[2].name, "Drop D");
    EXPECT_EQ(presets[3].name, "Drop C");
    EXPECT_EQ(presets[4].name, "DADGAD");
    EXPECT_EQ(presets[5].name, "Open G");
    EXPECT_EQ(presets[6].name, "Open D");
}

TEST(TuningPresetsTest, FindClosestString_BoundaryConditions)
{
    // Test frequency exactly at tolerance boundary (25 cents from E2)
    float boundaryFreq = 82.41f * std::pow(2.0f, 24.9f / 1200.0f);
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, boundaryFreq, 440.0f, 25.0f);
    EXPECT_TRUE(result.has_value()); // Should match within tolerance
    
    // Test frequency just outside tolerance (25.1 cents from E2)
    float outsideFreq = 82.41f * std::pow(2.0f, 25.1f / 1200.0f);
    result = TuningPresets::FindClosestString(TuningMode::Standard, outsideFreq, 440.0f, 25.0f);
    EXPECT_FALSE(result.has_value()); // Should not match
}

TEST(TuningPresetsTest, FindClosestString_BetweenStrings)
{
    // Test frequency exactly between E2 (82.41 Hz) and A2 (110.00 Hz)
    // This should find the closer one based on cents
    float midFreq = 95.0f; // Approximately between E2 and A2
    
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, midFreq, 440.0f, 25.0f);
    
    // Calculate which is closer in cents
    float centsToE2 = std::abs(GuitarDSP::NoteConverter::FrequencyToCents(midFreq, 82.41f));
    float centsToA2 = std::abs(GuitarDSP::NoteConverter::FrequencyToCents(midFreq, 110.00f));
    
    if (centsToE2 < 25.0f || centsToA2 < 25.0f)
    {
        EXPECT_TRUE(result.has_value());
    }
    else
    {
        EXPECT_FALSE(result.has_value());
    }
}
