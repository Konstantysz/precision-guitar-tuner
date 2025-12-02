#pragma once

#include <cstdint>

namespace PrecisionTuner::Constants
{
    /**
     * @brief Global application constants
     *
     * This file contains all global constants used throughout the application.
     * Constants use bastardized Hungarian notation for clarity:
     *  - k prefix for constants
     *  - Type indicators: u (unsigned), f (float), etc.
     */

    // ===== Audio Processing Constants =====

    /// Buffer allocation safety margin multiplier
    static constexpr uint32_t kuBufferSafetyMultiplier = 4;

    // ===== Tuner Visualization Constants =====

    /// Threshold for "in tune" indication (cents)
    static constexpr float kfInTuneThresholdCents = 3.0f;

    /// Tolerance for finding closest string (cents)
    static constexpr float kfTargetStringToleranceCents = 25.0f;

    /// Gauge radius as fraction of window size
    static constexpr float kfGaugeRadiusScale = 0.38f;

    // ===== Settings Layer Constants =====

    /// Minimum reference frequency for slider (Hz)
    static constexpr float kfMinReferenceFrequencyHz = 100.0f;

    /// Maximum reference frequency for slider (Hz)
    static constexpr float kfMaxReferenceFrequencyHz = 1000.0f;

} // namespace PrecisionTuner::Constants
