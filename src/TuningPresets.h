#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <Config.h>

namespace PrecisionTuner
{

    /**
     * @brief Tuning preset containing target frequencies for all 6 strings
     */
    struct TuningPreset
    {
        std::string name;                       ///< Preset name (e.g., "Standard E")
        std::array<float, 6> targetFrequencies; ///< Low E (6th) to High E (1st)
        std::array<std::string, 6> noteNames;   ///< Display names (e.g., "E2", "A2")
    };

    /**
     * @brief Manages guitar tuning presets and target string identification
     *
     * Provides pre-calculated frequency targets for standard and alternate guitar tunings,
     * with support for custom reference pitch (A4 = 430-450 Hz).
     */
    class TuningPresets
    {
    public:
        /**
         * @brief Get tuning preset for specified mode
         * @param mode Tuning mode (Chromatic, Standard, Drop D, etc.)
         * @param referencePitch A4 reference frequency in Hz (default 440.0)
         * @return Tuning preset with calculated frequencies
         */
        [[nodiscard]] static TuningPreset GetPreset(TuningMode mode, float referencePitch = 440.0f);

        /**
         * @brief Get all available tuning presets
         * @param referencePitch A4 reference frequency in Hz (default 440.0)
         * @return Vector of all tuning presets
         */
        [[nodiscard]] static std::vector<TuningPreset> GetAllPresets(float referencePitch = 440.0f);

        /**
         * @brief Find closest target string for detected frequency
         *
         * Identifies which guitar string the detected frequency is closest to,
         * within a specified tolerance. Returns nullopt for chromatic mode or
         * if no string is within tolerance.
         *
         * @param mode Tuning mode
         * @param frequency Detected frequency in Hz
         * @param referencePitch A4 reference frequency in Hz
         * @param toleranceCents Maximum deviation in cents (default 25)
         * @return String index (0=low E/6th, 5=high E/1st) or nullopt
         */
        [[nodiscard]] static std::optional<int> FindClosestString(TuningMode mode,
            float frequency,
            float referencePitch = 440.0f,
            float toleranceCents = 25.0f);

        /**
         * @brief Get formatted string name for display
         * @param stringIndex String index (0-5, where 0 = 6th string)
         * @param tuningMode Tuning mode for note name
         * @param referencePitch A4 reference frequency in Hz
         * @return Formatted string (e.g., "6th String (E2)", "1st String (E4)")
         */
        [[nodiscard]] static std::string
            GetStringName(int stringIndex, TuningMode tuningMode, float referencePitch = 440.0f);

    private:
        /**
         * @brief Preset definition with note names and octaves
         */
        struct PresetDefinition
        {
            std::string name;                     ///< Preset name
            std::array<std::string, 6> noteNames; ///< Note names without octave
            std::array<int, 6> octaves;           ///< Octave numbers
        };

        /**
         * @brief Calculate frequencies for a preset definition
         * @param definition Preset with note names and octaves
         * @param referencePitch A4 reference frequency in Hz
         * @return Tuning preset with calculated frequencies
         */
        [[nodiscard]] static TuningPreset CalculatePreset(const PresetDefinition &definition, float referencePitch);

        static const std::array<PresetDefinition, 7> presetDefinitions; ///< Array of preset definitions
    };

} // namespace PrecisionTuner
