/**
 * Tuning Presets Implementation
 * Calculates target frequencies for various guitar tunings
 *
 * Copyright (c) 2025
 * Licensed under the MIT License
 */

#include "TuningPresets.h"
#include <Logger.h>
#include <cmath>
#include <format>
#include <NoteConverter.h>

namespace PrecisionTuner
{

    // Tuning definitions: note names and octaves for each of the 6 strings
    const std::array<TuningPresets::PresetDefinition, 7> TuningPresets::presetDefinitions = { {
        { "Chromatic", {}, {} },
        { "Standard (EADGBE)", { "E", "A", "D", "G", "B", "E" }, { 2, 2, 3, 3, 3, 4 } },
        { "Drop D", { "D", "A", "D", "G", "B", "E" }, { 2, 2, 3, 3, 3, 4 } },
        { "Drop C", { "C", "G", "C", "F", "A", "D" }, { 2, 2, 3, 3, 3, 4 } },
        { "DADGAD", { "D", "A", "D", "G", "A", "D" }, { 2, 2, 3, 3, 3, 4 } },
        { "Open G", { "D", "G", "D", "G", "B", "D" }, { 2, 2, 3, 3, 3, 4 } },
        { "Open D", { "D", "A", "D", "F#", "A", "D" }, { 2, 2, 3, 3, 3, 4 } },
    } };

    TuningPreset TuningPresets::CalculatePreset(const PresetDefinition &definition, float referencePitch)
    {
        TuningPreset preset;
        preset.name = definition.name;

        // Calculate frequencies for each string
        for (size_t i = 0; i < 6; ++i)
        {
            if (!definition.noteNames[i].empty())
            {
                preset.targetFrequencies[i] = GuitarDSP::NoteConverter::NoteToFrequency(
                    definition.noteNames[i], definition.octaves[i], referencePitch);
                preset.noteNames[i] = definition.noteNames[i] + std::to_string(definition.octaves[i]);
            }
            else
            {
                // Chromatic mode has no target frequencies
                preset.targetFrequencies[i] = 0.0f;
                preset.noteNames[i] = "";
            }
        }

        return preset;
    }

    TuningPreset TuningPresets::GetPreset(TuningMode mode, float referencePitch)
    {
        size_t index = static_cast<size_t>(mode);
        if (index >= presetDefinitions.size())
        {
            LOG_WARN("Invalid tuning mode index: {}. Defaulting to Chromatic.", index);
            index = 0;
        }

        return CalculatePreset(presetDefinitions[index], referencePitch);
    }

    std::vector<TuningPreset> TuningPresets::GetAllPresets(float referencePitch)
    {
        std::vector<TuningPreset> presets;
        presets.reserve(presetDefinitions.size());

        for (const auto &definition : presetDefinitions)
        {
            presets.push_back(CalculatePreset(definition, referencePitch));
        }

        return presets;
    }

    std::optional<int>
        TuningPresets::FindClosestString(TuningMode mode, float frequency, float referencePitch, float toleranceCents)
    {
        // Chromatic mode has no target strings
        if (mode == TuningMode::Chromatic)
        {
            return std::nullopt;
        }

        const auto preset = GetPreset(mode, referencePitch);

        int closestIndex = -1;
        float minCentsDiff = toleranceCents;

        // Find string with minimum cent deviation
        for (int i = 0; i < 6; ++i)
        {
            float centsDiff =
                std::abs(GuitarDSP::NoteConverter::FrequencyToCents(frequency, preset.targetFrequencies[i]));

            if (centsDiff < minCentsDiff)
            {
                minCentsDiff = centsDiff;
                closestIndex = i;
            }
        }

        if (closestIndex >= 0)
        {
            return closestIndex;
        }

        return std::nullopt;
    }

    std::string TuningPresets::GetStringName(int stringIndex, TuningMode tuningMode, float referencePitch)
    {
        if (stringIndex < 0 || stringIndex > 5)
        {
            return "Unknown String";
        }

        const auto preset = GetPreset(tuningMode, referencePitch);

        // String numbering: 6th string = index 0 (low E), 1st string = index 5 (high E)
        int displayNumber = 6 - stringIndex;
        const std::string &noteName = preset.noteNames[stringIndex];

        if (noteName.empty())
        {
            return std::format("{}th String", displayNumber);
        }

        // Format: "6th String (E2)" or "1st String (E4)"
        const char *suffix = (displayNumber == 1)   ? "st"
                             : (displayNumber == 2) ? "nd"
                             : (displayNumber == 3) ? "rd"
                                                    : "th";

        return std::format("{}{} String ({})", displayNumber, suffix, noteName);
    }

} // namespace PrecisionTuner
