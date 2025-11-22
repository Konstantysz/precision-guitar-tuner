#include "TunerVisualizationLayer.h"
#include <Logger.h>
#include <format>

TunerVisualizationLayer::TunerVisualizationLayer(AudioProcessingLayer &audioLayer) : audioLayer_(audioLayer)
{
    LOG_INFO("TunerVisualizationLayer - Initializing UI");
}

void TunerVisualizationLayer::OnUpdate(float deltaTime)
{
    updateTimer_ += deltaTime;

    // Update UI at fixed interval to avoid excessive logging
    if (updateTimer_ >= UPDATE_INTERVAL)
    {
        updateTimer_ = 0.0f;

        // Get latest pitch data from audio layer
        auto pitchData = audioLayer_.GetLatestPitch();

        if (pitchData.detected && pitchData.confidence > 0.7f)
        {
            // Convert frequency to note
            currentNote_ = GuitarDSP::NoteConverter::FrequencyToNote(pitchData.frequency);

            // Log detected pitch (temporary - will be replaced with OpenGL rendering)
            LOG_INFO("Detected: {}{} ({:.2f} Hz) | Deviation: {:+.1f} cents | Confidence: {:.0f}%",
                currentNote_.name,
                currentNote_.octave,
                pitchData.frequency,
                currentNote_.cents,
                pitchData.confidence * 100.0f);
        }
    }
}

void TunerVisualizationLayer::OnRender()
{
    // TODO: Render OpenGL tuner visualization
    // For now, just clear the screen to a dark background
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);

    // Future rendering:
    // - Note name display (large text)
    // - Frequency display
    // - Cent deviation meter (visual indicator)
    // - Tuning accuracy indicator (green when in tune)
}
