/**
 * Unit Tests for TuningPresets
 * Tests frequency calculations, string detection, and edge cases
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include <TuningPresets.h>
#include <NoteConverter.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <format>

using namespace PrecisionTuner;

// Test tolerance for frequency comparisons (Hz)
constexpr float FREQ_TOLERANCE = 0.01f;

// Test tolerance for cent comparisons
constexpr float CENTS_TOLERANCE = 0.1f;

void TestStandardTuning_A440_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 440.0f);
    
    assert(preset.name == "Standard (EADGBE)");
    
    // Expected frequencies at A440
    assert(std::abs(preset.targetFrequencies[0] - 82.41f) < FREQ_TOLERANCE);   // E2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f) < FREQ_TOLERANCE);  // A2
    assert(std::abs(preset.targetFrequencies[2] - 146.83f) < FREQ_TOLERANCE);  // D3
    assert(std::abs(preset.targetFrequencies[3] - 196.00f) < FREQ_TOLERANCE);  // G3
    assert(std::abs(preset.targetFrequencies[4] - 246.94f) < FREQ_TOLERANCE);  // B3
    assert(std::abs(preset.targetFrequencies[5] - 329.63f) < FREQ_TOLERANCE);  // E4
    
    // Verify note names
    assert(preset.noteNames[0] == "E2");
    assert(preset.noteNames[1] == "A2");
    assert(preset.noteNames[2] == "D3");
    assert(preset.noteNames[3] == "G3");
    assert(preset.noteNames[4] == "B3");
    assert(preset.noteNames[5] == "E4");
    
    std::cout << "TestStandardTuning_A440_CorrectFrequencies passed" << std::endl;
}

void TestStandardTuning_A442_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 442.0f);
    
    // At A442, frequencies should scale proportionally
    // A2 = 110Hz at A440, should be 110 * (442/440) = 110.5Hz at A442
    float scaleFactor = 442.0f / 440.0f;
    
    assert(std::abs(preset.targetFrequencies[0] - 82.41f * scaleFactor) < FREQ_TOLERANCE);   // E2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f * scaleFactor) < FREQ_TOLERANCE);  // A2
    assert(std::abs(preset.targetFrequencies[2] - 146.83f * scaleFactor) < FREQ_TOLERANCE);  // D3
    assert(std::abs(preset.targetFrequencies[3] - 196.00f * scaleFactor) < FREQ_TOLERANCE);  // G3
    assert(std::abs(preset.targetFrequencies[4] - 246.94f * scaleFactor) < FREQ_TOLERANCE);  // B3
    assert(std::abs(preset.targetFrequencies[5] - 329.63f * scaleFactor) < FREQ_TOLERANCE);  // E4
    
    std::cout << "TestStandardTuning_A442_CorrectFrequencies passed" << std::endl;
}

void TestDropD_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::DropD, 440.0f);
    
    assert(preset.name == "Drop D");
    
    // Drop D: D-A-D-G-B-E
    // 6th string (E2) drops to D2 (73.42 Hz)
    assert(std::abs(preset.targetFrequencies[0] - 73.42f) < FREQ_TOLERANCE);   // D2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f) < FREQ_TOLERANCE);  // A2
    assert(std::abs(preset.targetFrequencies[2] - 146.83f) < FREQ_TOLERANCE);  // D3
    assert(std::abs(preset.targetFrequencies[3] - 196.00f) < FREQ_TOLERANCE);  // G3
    assert(std::abs(preset.targetFrequencies[4] - 246.94f) < FREQ_TOLERANCE);  // B3
    assert(std::abs(preset.targetFrequencies[5] - 329.63f) < FREQ_TOLERANCE);  // E4
    
    std::cout << "TestDropD_CorrectFrequencies passed" << std::endl;
}

void TestDropC_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::DropC, 440.0f);
    
    assert(preset.name == "Drop C");
    
    // Drop C: C-G-C-F-A-D
    assert(std::abs(preset.targetFrequencies[0] - 65.41f) < FREQ_TOLERANCE);   // C2
    assert(std::abs(preset.targetFrequencies[1] - 98.00f) < FREQ_TOLERANCE);   // G2
    assert(std::abs(preset.targetFrequencies[2] - 130.81f) < FREQ_TOLERANCE);  // C3
    assert(std::abs(preset.targetFrequencies[3] - 174.61f) < FREQ_TOLERANCE);  // F3
    assert(std::abs(preset.targetFrequencies[4] - 220.00f) < FREQ_TOLERANCE);  // A3
    assert(std::abs(preset.targetFrequencies[5] - 293.66f) < FREQ_TOLERANCE);  // D4
    
    std::cout << "TestDropC_CorrectFrequencies passed" << std::endl;
}

void TestDADGAD_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::DADGAD, 440.0f);
    
    assert(preset.name == "DADGAD");
    
    // DADGAD: D-A-D-G-A-D
    assert(std::abs(preset.targetFrequencies[0] - 73.42f) < FREQ_TOLERANCE);   // D2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f) < FREQ_TOLERANCE);  // A2
    assert(std::abs(preset.targetFrequencies[2] - 146.83f) < FREQ_TOLERANCE);  // D3
    assert(std::abs(preset.targetFrequencies[3] - 196.00f) < FREQ_TOLERANCE);  // G3
    assert(std::abs(preset.targetFrequencies[4] - 220.00f) < FREQ_TOLERANCE);  // A3
    assert(std::abs(preset.targetFrequencies[5] - 293.66f) < FREQ_TOLERANCE);  // D4
    
    std::cout << "TestDADGAD_CorrectFrequencies passed" << std::endl;
}

void TestOpenG_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::OpenG, 440.0f);
    
    assert(preset.name == "Open G");
    
    // Open G: D-G-D-G-B-D
    assert(std::abs(preset.targetFrequencies[0] - 73.42f) < FREQ_TOLERANCE);   // D2
    assert(std::abs(preset.targetFrequencies[1] - 98.00f) < FREQ_TOLERANCE);   // G2
    assert(std::abs(preset.targetFrequencies[2] - 146.83f) < FREQ_TOLERANCE);  // D3
    assert(std::abs(preset.targetFrequencies[3] - 196.00f) < FREQ_TOLERANCE);  // G3
    assert(std::abs(preset.targetFrequencies[4] - 246.94f) < FREQ_TOLERANCE);  // B3
    assert(std::abs(preset.targetFrequencies[5] - 293.66f) < FREQ_TOLERANCE);  // D4
    
    std::cout << "TestOpenG_CorrectFrequencies passed" << std::endl;
}

void TestOpenD_CorrectFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::OpenD, 440.0f);
    
    assert(preset.name == "Open D");
    
    // Open D: D-A-D-F#-A-D
    assert(std::abs(preset.targetFrequencies[0] - 73.42f) < FREQ_TOLERANCE);   // D2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f) < FREQ_TOLERANCE);  // A2
    assert(std::abs(preset.targetFrequencies[2] - 146.83f) < FREQ_TOLERANCE);  // D3
    assert(std::abs(preset.targetFrequencies[3] - 185.00f) < FREQ_TOLERANCE);  // F#3
    assert(std::abs(preset.targetFrequencies[4] - 220.00f) < FREQ_TOLERANCE);  // A3
    assert(std::abs(preset.targetFrequencies[5] - 293.66f) < FREQ_TOLERANCE);  // D4
    
    std::cout << "TestOpenD_CorrectFrequencies passed" << std::endl;
}

void TestChromaticMode_NoTargetFrequencies()
{
    auto preset = TuningPresets::GetPreset(TuningMode::Chromatic, 440.0f);
    
    assert(preset.name == "Chromatic");
    
    // Chromatic mode should have zero frequencies
    for (size_t i = 0; i < 6; ++i)
    {
        assert(preset.targetFrequencies[i] == 0.0f);
        assert(preset.noteNames[i].empty());
    }
    
    std::cout << "TestChromaticMode_NoTargetFrequencies passed" << std::endl;
}

void TestFindClosestString_ExactMatch()
{
    // Test exact frequency match for standard tuning
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, 82.41f, 440.0f, 25.0f);
    
    assert(result.has_value());
    assert(result.value() == 0); // 6th string (E2)
    
    result = TuningPresets::FindClosestString(TuningMode::Standard, 110.00f, 440.0f, 25.0f);
    assert(result.has_value());
    assert(result.value() == 1); // 5th string (A2)
    
    result = TuningPresets::FindClosestString(TuningMode::Standard, 329.63f, 440.0f, 25.0f);
    assert(result.has_value());
    assert(result.value() == 5); // 1st string (E4)
    
    std::cout << "TestFindClosestString_ExactMatch passed" << std::endl;
}

void TestFindClosestString_WithinTolerance()
{
    // Test frequency slightly sharp (+10 cents from E2)
    // 10 cents = frequency * 2^(10/1200)
    float sharpE2 = 82.41f * std::pow(2.0f, 10.0f / 1200.0f); // ~82.89 Hz
    
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, sharpE2, 440.0f, 25.0f);
    assert(result.has_value());
    assert(result.value() == 0); // Should still match 6th string
    
    // Test frequency slightly flat (-15 cents from A2)
    float flatA2 = 110.00f * std::pow(2.0f, -15.0f / 1200.0f); // ~109.04 Hz
    
    result = TuningPresets::FindClosestString(TuningMode::Standard, flatA2, 440.0f, 25.0f);
    assert(result.has_value());
    assert(result.value() == 1); // Should still match 5th string
    
    std::cout << "TestFindClosestString_WithinTolerance passed" << std::endl;
}

void TestFindClosestString_OutsideTolerance()
{
    // Test frequency way too sharp (50 cents from E2, outside default 25-cent tolerance)
    float verySharpE2 = 82.41f * std::pow(2.0f, 50.0f / 1200.0f); // ~85.02 Hz
    
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, verySharpE2, 440.0f, 25.0f);
    assert(!result.has_value()); // Should not match any string
    
    // Test with extremely low frequency
    result = TuningPresets::FindClosestString(TuningMode::Standard, 50.0f, 440.0f, 25.0f);
    assert(!result.has_value());
    
    // Test with extremely high frequency
    result = TuningPresets::FindClosestString(TuningMode::Standard, 500.0f, 440.0f, 25.0f);
    assert(!result.has_value());
    
    std::cout << "TestFindClosestString_OutsideTolerance passed" << std::endl;
}

void TestFindClosestString_ChromaticMode()
{
    // Chromatic mode should always return nullopt
    auto result = TuningPresets::FindClosestString(TuningMode::Chromatic, 82.41f, 440.0f, 25.0f);
    assert(!result.has_value());
    
    result = TuningPresets::FindClosestString(TuningMode::Chromatic, 440.0f, 440.0f, 25.0f);
    assert(!result.has_value());
    
    std::cout << "TestFindClosestString_ChromaticMode passed" << std::endl;
}

void TestGetStringName_StandardTuning()
{
    // Test all string indices with standard tuning
    assert(TuningPresets::GetStringName(0, TuningMode::Standard) == "6th String (E2)");
    assert(TuningPresets::GetStringName(1, TuningMode::Standard) == "5th String (A2)");
    assert(TuningPresets::GetStringName(2, TuningMode::Standard) == "4th String (D3)");
    assert(TuningPresets::GetStringName(3, TuningMode::Standard) == "3rd String (G3)");
    assert(TuningPresets::GetStringName(4, TuningMode::Standard) == "2nd String (B3)");
    assert(TuningPresets::GetStringName(5, TuningMode::Standard) == "1st String (E4)");
    
    std::cout << "TestGetStringName_StandardTuning passed" << std::endl;
}

void TestGetStringName_DropD()
{
    // Test Drop D tuning (first string is D2 instead of E2)
    assert(TuningPresets::GetStringName(0, TuningMode::DropD) == "6th String (D2)");
    assert(TuningPresets::GetStringName(5, TuningMode::DropD) == "1st String (E4)");
    
    std::cout << "TestGetStringName_DropD passed" << std::endl;
}

void TestGetStringName_InvalidIndices()
{
    // Test invalid indices
    assert(TuningPresets::GetStringName(-1, TuningMode::Standard) == "Unknown String");
    assert(TuningPresets::GetStringName(6, TuningMode::Standard) == "Unknown String");
    assert(TuningPresets::GetStringName(100, TuningMode::Standard) == "Unknown String");
    
    std::cout << "TestGetStringName_InvalidIndices passed" << std::endl;
}

void TestReferencePitchAdjustment_A430()
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 430.0f);
    
    // At A430, frequencies should scale proportionally
    float scaleFactor = 430.0f / 440.0f;
    
    assert(std::abs(preset.targetFrequencies[0] - 82.41f * scaleFactor) < FREQ_TOLERANCE);   // E2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f * scaleFactor) < FREQ_TOLERANCE);  // A2
    
    std::cout << "TestReferencePitchAdjustment_A430 passed" << std::endl;
}

void TestReferencePitchAdjustment_A450()
{
    auto preset = TuningPresets::GetPreset(TuningMode::Standard, 450.0f);
    
    // At A450, frequencies should scale proportionally
    float scaleFactor = 450.0f / 440.0f;
    
    assert(std::abs(preset.targetFrequencies[0] - 82.41f * scaleFactor) < FREQ_TOLERANCE);   // E2
    assert(std::abs(preset.targetFrequencies[1] - 110.00f * scaleFactor) < FREQ_TOLERANCE);  // A2
    
    std::cout << "TestReferencePitchAdjustment_A450 passed" << std::endl;
}

void TestGetAllPresets()
{
    auto presets = TuningPresets::GetAllPresets(440.0f);
    
    assert(presets.size() == 7);
    assert(presets[0].name == "Chromatic");
    assert(presets[1].name == "Standard (EADGBE)");
    assert(presets[2].name == "Drop D");
    assert(presets[3].name == "Drop C");
    assert(presets[4].name == "DADGAD");
    assert(presets[5].name == "Open G");
    assert(presets[6].name == "Open D");
    
    std::cout << "TestGetAllPresets passed" << std::endl;
}

void TestFindClosestString_BoundaryConditions()
{
    // Test frequency exactly at tolerance boundary (25 cents from E2)
    float boundaryFreq = 82.41f * std::pow(2.0f, 24.9f / 1200.0f);
    auto result = TuningPresets::FindClosestString(TuningMode::Standard, boundaryFreq, 440.0f, 25.0f);
    assert(result.has_value()); // Should match within tolerance
    
    // Test frequency just outside tolerance (25.1 cents from E2)
    float outsideFreq = 82.41f * std::pow(2.0f, 25.1f / 1200.0f);
    result = TuningPresets::FindClosestString(TuningMode::Standard, outsideFreq, 440.0f, 25.0f);
    assert(!result.has_value()); // Should not match
    
    std::cout << "TestFindClosestString_BoundaryConditions passed" << std::endl;
}

void TestFindClosestString_BetweenStrings()
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
        assert(result.has_value());
    }
    else
    {
        assert(!result.has_value());
    }
    
    std::cout << "TestFindClosestString_BetweenStrings passed" << std::endl;
}

int main()
{
    std::cout << "Running TuningPresets Tests..." << std::endl;
    std::cout << std::endl;
    
    // Frequency calculation tests
    TestStandardTuning_A440_CorrectFrequencies();
    TestStandardTuning_A442_CorrectFrequencies();
    TestDropD_CorrectFrequencies();
    TestDropC_CorrectFrequencies();
    TestDADGAD_CorrectFrequencies();
    TestOpenG_CorrectFrequencies();
    TestOpenD_CorrectFrequencies();
    TestChromaticMode_NoTargetFrequencies();
    
    // String detection tests
    TestFindClosestString_ExactMatch();
    TestFindClosestString_WithinTolerance();
    TestFindClosestString_OutsideTolerance();
    TestFindClosestString_ChromaticMode();
    TestFindClosestString_BoundaryConditions();
    TestFindClosestString_BetweenStrings();
    
    // String name tests
    TestGetStringName_StandardTuning();
    TestGetStringName_DropD();
    TestGetStringName_InvalidIndices();
    
    // Reference pitch adjustment tests
    TestReferencePitchAdjustment_A430();
    TestReferencePitchAdjustment_A450();
    
    // Additional tests
    TestGetAllPresets();
    
    std::cout << std::endl;
    std::cout << "All 21 TuningPresets tests passed!" << std::endl;
    return 0;
}
