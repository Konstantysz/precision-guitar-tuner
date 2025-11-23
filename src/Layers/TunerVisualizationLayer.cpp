#include <TunerVisualizationLayer.h>
#include <AudioProcessingLayer.h>
#include <Logger.h>
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <format>

namespace PrecisionTuner::Layers
{

TunerVisualizationLayer::TunerVisualizationLayer(AudioProcessingLayer &audioLayer) : audioLayer(audioLayer)
{
    LOG_INFO("TunerVisualizationLayer - Initializing ImGui-based tuner UI");
}

void TunerVisualizationLayer::OnUpdate(float deltaTime)
{
    updateTimer += deltaTime;

    // Update UI at fixed interval to avoid excessive logging
    if (updateTimer >= UPDATE_INTERVAL)
    {
        updateTimer = 0.0f;

        // Get latest pitch data from audio layer
        auto pitchData = audioLayer.GetLatestPitch();

        if (pitchData.detected && pitchData.confidence > 0.7f)
        {
            // Convert frequency to note
            currentNote = GuitarDSP::NoteConverter::FrequencyToNote(pitchData.frequency);
            hasPitchData = true;

            // Log detected pitch
            LOG_INFO("Detected: {}{} ({:.2f} Hz) | Deviation: {:+.1f} cents | Confidence: {:.0f}%",
                currentNote.name,
                currentNote.octave,
                pitchData.frequency,
                currentNote.cents,
                pitchData.confidence * 100.0f);
        }
        else
        {
            hasPitchData = false;
        }
    }
}

void TunerVisualizationLayer::OnRender()
{
    // Create main tuner window (fullscreen, no titlebar)
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.12f, 1.0f)); // Dark background

    if (ImGui::Begin("Tuner", nullptr, flags))
    {
        // Render settings toggle button in top-right corner
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(windowSize.x - 150.0f, 10.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.4f, 0.6f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.5f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.6f, 0.8f, 1.0f));

        if (ImGui::Button(showSettingsPanel ? "Hide Settings" : "Show Settings", ImVec2(140.0f, 30.0f)))
        {
            showSettingsPanel = !showSettingsPanel;
        }

        ImGui::PopStyleColor(3);

        // Render tuner UI elements
        RenderTuningIndicator();
        RenderCentDeviationMeter();
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void TunerVisualizationLayer::RenderTuningIndicator()
{
    if (!hasPitchData)
    {
        // Show "No signal" message when no pitch detected
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetCursorPos(ImVec2(windowSize.x * 0.5f - 100.0f, windowSize.y * 0.3f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.55f, 1.0f));
        ImGui::Text("No signal detected");
        ImGui::PopStyleColor();
        return;
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 center(windowSize.x * 0.5f, windowSize.y * 0.3f);

    // Get color based on cent deviation
    ImVec4 color = GetColorForCents(currentNote.cents);
    ImU32 colorU32 = ImGui::ColorConvertFloat4ToU32(color);

    // Draw circular indicator
    const float indicatorRadius = 80.0f;
    drawList->AddCircleFilled(center, indicatorRadius, colorU32, 64);
    drawList->AddCircle(center, indicatorRadius, IM_COL32(255, 255, 255, 255), 64, 3.0f);

    // Draw "IN TUNE" indicator when within ±3 cents
    if (std::abs(currentNote.cents) <= 3.0f)
    {
        // Pulsing border
        const float pulseRadius = indicatorRadius * 1.3f;
        drawList->AddCircle(center, pulseRadius, IM_COL32(51, 255, 77, 255), 64, 4.0f);

        // Checkmark below circle
        ImVec2 checkStart(center.x - 40.0f, center.y + indicatorRadius + 30.0f);
        ImVec2 checkEnd(center.x + 40.0f, center.y + indicatorRadius + 30.0f);
        drawList->AddRectFilled(checkStart, checkEnd, IM_COL32(51, 255, 77, 255), 10.0f);
    }

    // Display note name above indicator (e.g., "E4")
    std::string noteText;
    noteText += currentNote.name;
    noteText += std::to_string(currentNote.octave);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font
    ImVec2 noteTextSize = ImGui::CalcTextSize(noteText.c_str());
    noteTextSize.x *= 2.0f; // Scale up
    noteTextSize.y *= 2.0f;

    ImGui::SetCursorPos(ImVec2(center.x - noteTextSize.x * 0.25f, center.y - indicatorRadius - 80.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(3.0f);
    ImGui::Text("%s", noteText.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    ImGui::PopFont();

    // Display frequency below indicator (e.g., "440.0 Hz")
    std::string freqText = std::format("{:.1f} Hz", currentNote.frequency);
    ImVec2 freqTextSize = ImGui::CalcTextSize(freqText.c_str());

    ImGui::SetCursorPos(ImVec2(center.x - freqTextSize.x * 0.6f, center.y + indicatorRadius + 60.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.85f, 1.0f));
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("%s", freqText.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    // Display cent deviation below frequency (e.g., "+2.3")
    std::string centsText = std::format("{:+.1f} cents", currentNote.cents);
    ImVec2 centsTextSize = ImGui::CalcTextSize(centsText.c_str());

    ImGui::SetCursorPos(ImVec2(center.x - centsTextSize.x * 0.8f, center.y + indicatorRadius + 90.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("%s", centsText.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
}

void TunerVisualizationLayer::RenderCentDeviationMeter()
{
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImDrawList *drawList = ImGui::GetWindowDrawList();

    // Meter dimensions and position
    const float meterWidth = windowSize.x * 0.8f;
    const float meterHeight = 60.0f;
    const float meterY = windowSize.y * 0.65f;
    const float meterX = (windowSize.x - meterWidth) * 0.5f;

    // Draw meter background (dark gray)
    ImVec2 meterMin(meterX, meterY);
    ImVec2 meterMax(meterX + meterWidth, meterY + meterHeight);
    drawList->AddRectFilled(meterMin, meterMax, IM_COL32(51, 51, 56, 255), 5.0f);

    // Draw center line (white)
    const float centerX = meterX + meterWidth * 0.5f;
    drawList->AddRectFilled(
        ImVec2(centerX - 2.0f, meterY),
        ImVec2(centerX + 2.0f, meterY + meterHeight),
        IM_COL32(255, 255, 255, 255)
    );

    // Draw tick marks at ±10, ±20, ±30, ±40, ±50 cents
    for (int cent : {-50, -40, -30, -20, -10, 10, 20, 30, 40, 50})
    {
        float x = centerX + (cent / 50.0f) * (meterWidth * 0.5f);
        float tickHeight = (cent % 20 == 0) ? meterHeight * 0.6f : meterHeight * 0.4f;
        float tickY = meterY + (meterHeight - tickHeight) * 0.5f;

        drawList->AddRectFilled(
            ImVec2(x - 1.0f, tickY),
            ImVec2(x + 1.0f, tickY + tickHeight),
            IM_COL32(128, 128, 140, 255)
        );
    }

    // Draw current cent deviation position (colored indicator)
    if (hasPitchData)
    {
        // Clamp cents to ±50 range for display
        float clampedCents = std::clamp(currentNote.cents, -50.0f, 50.0f);
        float indicatorX = centerX + (clampedCents / 50.0f) * (meterWidth * 0.5f);

        ImVec4 color = GetColorForCents(currentNote.cents);
        ImU32 colorU32 = ImGui::ColorConvertFloat4ToU32(color);

        drawList->AddRectFilled(
            ImVec2(indicatorX - 8.0f, meterY - 10.0f),
            ImVec2(indicatorX + 8.0f, meterY + meterHeight + 10.0f),
            colorU32,
            5.0f
        );
    }

    // Draw meter outline (white)
    drawList->AddRect(meterMin, meterMax, IM_COL32(153, 153, 166, 255), 5.0f, 0, 2.0f);
}

ImVec4 TunerVisualizationLayer::GetColorForCents(float cents)
{
    float absCents = std::abs(cents);

    if (absCents <= 3.0f)
    {
        // In tune - green
        return ImVec4(0.2f, 0.9f, 0.3f, 1.0f);
    }
    else if (absCents <= 10.0f)
    {
        // Close - yellow-green blend
        float t = (absCents - 3.0f) / 7.0f;
        return ImVec4(
            0.2f + (0.7f * t),  // R: 0.2 -> 0.9
            0.9f,                // G: 0.9
            0.3f - (0.1f * t),  // B: 0.3 -> 0.2
            1.0f
        );
    }
    else if (absCents <= 25.0f)
    {
        // Getting far - yellow to orange
        float t = (absCents - 10.0f) / 15.0f;
        return ImVec4(
            0.9f,                // R: 0.9
            0.9f - (0.4f * t),  // G: 0.9 -> 0.5
            0.2f - (0.1f * t),  // B: 0.2 -> 0.1
            1.0f
        );
    }
    else
    {
        // Very out of tune - red
        return ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
    }
}

} // namespace PrecisionTuner::Layers
