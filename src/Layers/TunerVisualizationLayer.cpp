#include "Constants.h"
#include <Logger.h>
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <AudioProcessingLayer.h>
#include <TunerVisualizationLayer.h>
#include <TuningPresets.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

namespace
{
    static constexpr float SMOOTHING_FACTOR = 10.0f; ///< Smoothing factor for cent display
    static constexpr float UPDATE_INTERVAL = 0.1f;   ///< UI update rate (100ms)
} // namespace

namespace PrecisionTuner::Layers
{
    TunerVisualizationLayer::TunerVisualizationLayer(AudioProcessingLayer &audioLayer, PrecisionTuner::Config &config)
        : audioLayer(audioLayer), config(config), currentNote(std::nullopt), updateTimer(0.0f), hasPitchData(false),
          showSettingsPanel(true), targetStringIndex(std::nullopt), smoothedCents(0.0f), woodBackgroundTexture(0),
          gaugeFaceTexture(0), chromeTexture(0)
    {
        LOG_INFO("TunerVisualizationLayer - Initializing tuner UI");
        InitializeTextures();
    }

    TunerVisualizationLayer::~TunerVisualizationLayer()
    {
        CleanupTextures();
    }

    bool TunerVisualizationLayer::IsSettingsVisible() const
    {
        return showSettingsPanel;
    }

    void TunerVisualizationLayer::SetSettingsVisible(bool visible)
    {
        showSettingsPanel = visible;
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
                // Convert frequency to note (using reference pitch from config)
                currentNote =
                    GuitarDSP::NoteConverter::FrequencyToNote(pitchData.frequency, config.tuning.referencePitch);
                hasPitchData = true;

                // Find target string in non-chromatic modes
                targetStringIndex = TuningPresets::FindClosestString(config.tuning.mode,
                    pitchData.frequency,
                    config.tuning.referencePitch,
                    Constants::kfTargetStringToleranceCents);

                // Log detected pitch
                LOG_INFO("Detected: {}{} ({:.2f} Hz) | Deviation: {:+.1f} cents | Confidence: {:.0f}%",
                    currentNote.value().name,
                    currentNote.value().octave,
                    pitchData.frequency,
                    currentNote.value().cents,
                    pitchData.confidence * 100.0f);
            }
            else
            {
                hasPitchData = false;
                targetStringIndex = std::nullopt;
            }
        }

        // Smooth the cents value for display
        float targetCents = hasPitchData ? currentNote.value().cents : 0.0f;
        // Use a simple lerp for smoothing
        smoothedCents += (targetCents - smoothedCents) * deltaTime * SMOOTHING_FACTOR;
    }

    void TunerVisualizationLayer::OnRender()
    {
        // Create main tuner window (fullscreen, no titlebar)
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        // Retro gauge background color (dark leather/wood desk feel behind the gauge)
        ImVec4 bgColor = ImVec4(0.15f, 0.12f, 0.1f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, bgColor);

        if (ImGui::Begin("Tuner", nullptr, flags))
        {
            // 1. Draw the Gauge and Background FIRST (so they are behind the UI)
            RenderRetroGauge();

            // 2. Render settings gear icon in top-right corner (ON TOP)
            float gearSize = 40.0f;
            float padding = 20.0f;
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 gearPos(windowSize.x - gearSize - padding, padding);

            ImGui::SetCursorPos(gearPos);

            // Invisible button to handle interactions
            if (ImGui::InvisibleButton("SettingsGear", ImVec2(gearSize, gearSize)))
            {
                showSettingsPanel = !showSettingsPanel;
            }

            // Draw Gear Icon
            bool isHovered = ImGui::IsItemHovered();
            bool isActive = ImGui::IsItemActive();

            ImDrawList *drawList = ImGui::GetWindowDrawList();
            ImVec2 center(gearPos.x + gearSize * 0.5f, gearPos.y + gearSize * 0.5f);
            ImU32 gearColor = isHovered ? (isActive ? IM_COL32(180, 180, 180, 255) : IM_COL32(255, 255, 255, 255))
                                        : IM_COL32(200, 200, 200, 200);

            // Gear Body
            float radius = gearSize * 0.35f;
            float toothLen = gearSize * 0.1f;
            int numTeeth = 8;

            // Draw teeth
            for (int i = 0; i < numTeeth; ++i)
            {
                float angle = (float)i / (float)numTeeth * 6.283185f;
                float nextAngle = (float)(i + 1) / (float)numTeeth * 6.283185f;
                float toothWidth = (nextAngle - angle) * 0.5f;

                ImVec2 p1(center.x + cos(angle - toothWidth * 0.5f) * (radius + toothLen),
                    center.y + sin(angle - toothWidth * 0.5f) * (radius + toothLen));
                ImVec2 p2(center.x + cos(angle + toothWidth * 0.5f) * (radius + toothLen),
                    center.y + sin(angle + toothWidth * 0.5f) * (radius + toothLen));
                ImVec2 p3(center.x + cos(angle + toothWidth * 0.5f) * radius,
                    center.y + sin(angle + toothWidth * 0.5f) * radius);
                ImVec2 p4(center.x + cos(angle - toothWidth * 0.5f) * radius,
                    center.y + sin(angle - toothWidth * 0.5f) * radius);

                drawList->AddQuadFilled(p1, p2, p3, p4, gearColor);
            }

            // Draw main circle
            drawList->AddCircleFilled(center, radius, gearColor);
            // Draw center hole
            drawList->AddCircleFilled(
                center, radius * 0.4f, IM_COL32(40, 30, 25, 255)); // Dark hole (matches background roughly)
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    void TunerVisualizationLayer::RenderRetroGauge()
    {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 center(windowSize.x * 0.5f, windowSize.y * 0.5f);

        // Gauge dimensions
        float gaugeRadius = std::min(windowSize.x, windowSize.y) * Constants::kfGaugeRadiusScale; // Slightly larger

        // LAYER 1: Wood Background (Scaled to COVER, not tile)
        if (woodBackgroundTexture)
        {
            // Calculate UVs for "Cover" style scaling to avoid tiling seams
            float texAspect = 1.0f; // Assuming square texture
            float winAspect = windowSize.x / windowSize.y;

            ImVec2 uvMin(0, 0), uvMax(1, 1);
            if (winAspect > texAspect)
            {
                // Window is wider than texture
                float scale = winAspect / texAspect;
                uvMin.y = (1.0f - 1.0f / scale) * 0.5f;
                uvMax.y = 1.0f - uvMin.y;
            }
            else
            {
                // Window is taller than texture
                float scale = texAspect / winAspect;
                uvMin.x = (1.0f - 1.0f / scale) * 0.5f;
                uvMax.x = 1.0f - uvMin.x;
            }

            drawList->AddImage(woodBackgroundTexture,
                windowPos,
                ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                uvMin,
                uvMax);

            // Vignette for focus
            drawList->AddRectFilledMultiColor(windowPos,
                ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                IM_COL32(0, 0, 0, 100),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 0),
                IM_COL32(0, 0, 0, 100));
        }

        // Drop shadow under the entire gauge unit
        drawList->AddCircleFilled(ImVec2(center.x, center.y + 10), gaugeRadius * 1.05f, IM_COL32(0, 0, 0, 120), 64);
        drawList->AddCircleFilled(ImVec2(center.x, center.y + 5), gaugeRadius * 1.02f, IM_COL32(0, 0, 0, 80), 64);

        // LAYER 2: Chrome Bezel (Base Layer)
        // We draw the chrome texture as a large circle. The face will be drawn on top, leaving a ring.
        float bezelWidth = gaugeRadius * 0.15f;
        float outerRadius = gaugeRadius;

        if (chromeTexture)
        {
            ImVec2 bezelMin(center.x - outerRadius, center.y - outerRadius);
            ImVec2 bezelMax(center.x + outerRadius, center.y + outerRadius);
            drawList->AddImageRounded(
                chromeTexture, bezelMin, bezelMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, outerRadius);

            // Add metallic shading overlay to give it 3D form (cylindrical shine)
            drawList->AddCircle(center,
                outerRadius - bezelWidth * 0.5f,
                IM_COL32(255, 255, 255, 60),
                128,
                bezelWidth * 0.2f); // Highlight
            drawList->AddCircle(center,
                outerRadius - bezelWidth * 0.1f,
                IM_COL32(0, 0, 0, 80),
                128,
                bezelWidth * 0.1f); // Outer rim shadow
        }
        else
        {
            drawList->AddCircleFilled(center, outerRadius, IM_COL32(150, 150, 150, 255), 128);
        }

        // LAYER 3: Gauge Face (Recessed)
        float faceRadius = gaugeRadius - bezelWidth;
        ImVec2 faceMin(center.x - faceRadius, center.y - faceRadius);
        ImVec2 faceMax(center.x + faceRadius, center.y + faceRadius);

        if (gaugeFaceTexture)
        {
            ImU32 tintColor = IM_COL32(255, 255, 255, 255);
            if (hasPitchData && std::abs(smoothedCents) < Constants::kfInTuneThresholdCents)
                tintColor = IM_COL32(230, 255, 230, 255);

            drawList->AddImageRounded(
                gaugeFaceTexture, faceMin, faceMax, ImVec2(0, 0), ImVec2(1, 1), tintColor, faceRadius);
        }
        else
        {
            drawList->AddCircleFilled(center, faceRadius, IM_COL32(245, 240, 220, 255), 128);
        }

        // Inner Shadow (to make face look recessed inside bezel)
        drawList->AddCircle(center, faceRadius, IM_COL32(0, 0, 0, 60), 128, 4.0f);
        drawList->AddCircle(center, faceRadius - 2, IM_COL32(0, 0, 0, 30), 128, 8.0f);

        // LAYER 4: Ticks & Labels
        const float angleRange = 120.0f * (3.14159f / 180.0f);
        const float startAngle = -3.14159f / 2.0f - (angleRange / 2.0f);

        int numTicks = 21;
        for (int i = 0; i < numTicks; ++i)
        {
            float t = (float)i / (float)(numTicks - 1);
            float angle = startAngle + t * angleRange;
            float centsVal = -50.0f + t * 100.0f;

            bool isMajor = (i % 2 == 0);
            bool isMain = (i % 10 == 0);

            float innerR = faceRadius * (isMain ? 0.75f : (isMajor ? 0.82f : 0.88f));
            float outerR = faceRadius * 0.92f;

            ImVec2 p1(center.x + cos(angle) * innerR, center.y + sin(angle) * innerR);
            ImVec2 p2(center.x + cos(angle) * outerR, center.y + sin(angle) * outerR);

            float thickness = isMain ? 3.0f : (isMajor ? 2.0f : 1.0f);
            drawList->AddLine(p1, p2, IM_COL32(20, 20, 20, 220), thickness);

            if (isMain)
            {
                std::string label = std::format("{:d}", (int)centsVal);
                ImVec2 labelSize = ImGui::CalcTextSize(label.c_str());
                float labelR = faceRadius * 0.63f;
                ImVec2 labelPos(center.x + cos(angle) * labelR - labelSize.x * 0.5f,
                    center.y + sin(angle) * labelR - labelSize.y * 0.5f);

                ImGui::SetCursorPos(ImVec2(labelPos.x - windowPos.x, labelPos.y - windowPos.y));
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(40, 30, 20, 255));
                ImGui::Text("%s", label.c_str());
                ImGui::PopStyleColor();
            }
        }

        // LAYER 5: 3D Needle
        {
            float displayCents = hasPitchData ? smoothedCents : 0.0f;
            float normalizedCents = std::clamp(displayCents, -50.0f, 50.0f) / 100.0f + 0.5f;
            float needleAngle = startAngle + normalizedCents * angleRange;

            float needleLen = faceRadius * 0.9f;
            float needleBase = faceRadius * 0.06f;

            // Calculate points
            ImVec2 tip(center.x + cos(needleAngle) * needleLen, center.y + sin(needleAngle) * needleLen);
            ImVec2 baseL(
                center.x + cos(needleAngle - 1.57f) * needleBase, center.y + sin(needleAngle - 1.57f) * needleBase);
            ImVec2 baseR(
                center.x + cos(needleAngle + 1.57f) * needleBase, center.y + sin(needleAngle + 1.57f) * needleBase);

            // Drop shadow for needle
            ImVec2 shadowOffset(3, 3);
            ImVec2 sTip(tip.x + shadowOffset.x, tip.y + shadowOffset.y);
            ImVec2 sBaseL(baseL.x + shadowOffset.x, baseL.y + shadowOffset.y);
            ImVec2 sBaseR(baseR.x + shadowOffset.x, baseR.y + shadowOffset.y);
            drawList->AddTriangleFilled(sTip, sBaseL, sBaseR, IM_COL32(0, 0, 0, 60));
            drawList->AddCircleFilled(
                ImVec2(center.x + shadowOffset.x, center.y + shadowOffset.y), needleBase, IM_COL32(0, 0, 0, 60));

            // Draw Needle in two halves for 3D ridge effect
            // Left half (Lighter)
            drawList->AddTriangleFilled(tip, baseL, center, IM_COL32(220, 50, 50, 255));
            // Right half (Darker)
            drawList->AddTriangleFilled(tip, center, baseR, IM_COL32(160, 30, 30, 255));

            // Pivot Cap (Chrome style)
            drawList->AddCircleFilled(center, needleBase * 0.8f, IM_COL32(200, 200, 200, 255)); // Silver
            drawList->AddCircleFilled(center, needleBase * 0.6f, IM_COL32(50, 50, 50, 255));    // Inner dark
            drawList->AddCircleFilled(center, needleBase * 0.3f, IM_COL32(230, 230, 230, 255)); // Highlight
        }

        // LAYER 6: Glass Reflection (Crisp & Clean - No Square Corners)
        {
            // Top glare - Soft circular highlight (simulating ceiling light on convex glass)
            // Offset upwards to create a "crescent" feel when combined with the face curvature
            drawList->AddCircleFilled(
                ImVec2(center.x, center.y - faceRadius * 0.35f), faceRadius * 0.55f, IM_COL32(255, 255, 255, 15));

            // Smaller, brighter hotspot for realism
            drawList->AddCircleFilled(
                ImVec2(center.x, center.y - faceRadius * 0.45f), faceRadius * 0.25f, IM_COL32(255, 255, 255, 15));

            // Bottom rim reflection (Sharp arc)
            // We draw an arc at the bottom to simulate the glass edge catching light
            drawList->PathArcTo(center, faceRadius * 0.94f, 0.2f, 2.94f); // Bottom arc
            drawList->PathStroke(IM_COL32(255, 255, 255, 25), 0, 2.0f);
        }

        // LAYER 7: Digital Display (Inset)
        float odoWidth = faceRadius * 0.9f;
        float odoHeight = faceRadius * 0.35f;
        float odoY = center.y + faceRadius * 0.45f;
        ImVec2 odoMin(center.x - odoWidth * 0.5f, odoY);
        ImVec2 odoMax(center.x + odoWidth * 0.5f, odoY + odoHeight);

        // Inset shadow
        drawList->AddRectFilled(odoMin, odoMax, IM_COL32(20, 20, 20, 255), 4.0f);
        drawList->AddRect(odoMin, odoMax, IM_COL32(100, 100, 100, 255), 4.0f);

        if (hasPitchData)
        {
            std::string noteStr = currentNote.value().name + std::to_string(currentNote.value().octave);
            ImGui::SetWindowFontScale(2.0f);
            ImVec2 noteSize = ImGui::CalcTextSize(noteStr.c_str());
            ImGui::SetCursorPos(ImVec2(center.x - noteSize.x * 0.5f - windowPos.x, odoY + 5 - windowPos.y));

            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForCents(smoothedCents));
            ImGui::Text("%s", noteStr.c_str());
            ImGui::PopStyleColor();
            ImGui::SetWindowFontScale(1.0f);

            std::string centsStr = std::format("{:+.1f} cents", smoothedCents);
            ImVec2 centsSize = ImGui::CalcTextSize(centsStr.c_str());
            ImGui::SetCursorPos(
                ImVec2(center.x - centsSize.x * 0.5f - windowPos.x, odoY + odoHeight * 0.6f - windowPos.y));
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", centsStr.c_str());
        }
        else
        {
            std::string text = "NO SIGNAL";
            ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            ImGui::SetCursorPos(
                ImVec2(center.x - textSize.x * 0.5f - windowPos.x, odoY + odoHeight * 0.35f - windowPos.y));
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "%s", text.c_str());
        }

        // Branding
        std::string brand = "PRECISION TUNER";
        ImGui::SetWindowFontScale(0.7f);
        ImVec2 brandSize = ImGui::CalcTextSize(brand.c_str());
        ImGui::SetCursorPos(
            ImVec2(center.x - brandSize.x * 0.5f - windowPos.x, center.y - faceRadius * 0.4f - windowPos.y));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(80, 70, 60, 200));
        ImGui::Text("%s", brand.c_str());
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
    }

    ImVec4 TunerVisualizationLayer::GetColorForCents(float cents)
    {
        float absCents = std::abs(cents);

        if (absCents <= Constants::kfInTuneThresholdCents)
        {
            // In tune - green
            return ImVec4(0.2f, 0.9f, 0.3f, 1.0f);
        }
        else if (absCents <= 10.0f)
        {
            // Close - yellow-green blend
            float t = (absCents - Constants::kfInTuneThresholdCents) / 7.0f;
            return ImVec4(0.2f + (0.7f * t), // R: 0.2 -> 0.9
                0.9f,                        // G: 0.9
                0.3f - (0.1f * t),           // B: 0.3 -> 0.2
                1.0f);
        }
        else if (absCents <= 25.0f)
        {
            // Getting far - yellow to orange
            float t = (absCents - 10.0f) / 15.0f;
            return ImVec4(0.9f,    // R: 0.9
                0.9f - (0.4f * t), // G: 0.9 -> 0.5
                0.2f - (0.1f * t), // B: 0.2 -> 0.1
                1.0f);
        }
        else
        {
            // Very out of tune - red
            return ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        }
    }

    ImTextureID TunerVisualizationLayer::LoadTexture(const char *path)
    {
        // Load image using stb_image
        int width, height, channels;
        unsigned char *data = stbi_load(path, &width, &height, &channels, 4); // Force RGBA

        if (!data)
        {
            LOG_ERROR("Failed to load texture: {}", path);
            return 0;
        }

        // Create OpenGL texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Upload texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Free image data
        stbi_image_free(data);

        LOG_INFO("Loaded texture: {} ({}x{}, {} channels)", path, width, height, channels);

        return (ImTextureID)(intptr_t)textureID;
    }

    void TunerVisualizationLayer::InitializeTextures()
    {
        LOG_INFO("Loading retro gauge textures...");

        woodBackgroundTexture = LoadTexture("assets/textures/wood_background.png");
        gaugeFaceTexture = LoadTexture("assets/textures/gauge_face.png");
        chromeTexture = LoadTexture("assets/textures/chrome_texture.png");

        LOG_INFO("Retro gauge textures loaded successfully");
    }

    void TunerVisualizationLayer::CleanupTextures()
    {
        // Delete OpenGL textures
        if (woodBackgroundTexture)
        {
            GLuint texID = (GLuint)(intptr_t)woodBackgroundTexture;
            glDeleteTextures(1, &texID);
        }
        if (gaugeFaceTexture)
        {
            GLuint texID = (GLuint)(intptr_t)gaugeFaceTexture;
            glDeleteTextures(1, &texID);
        }
        if (chromeTexture)
        {
            GLuint texID = (GLuint)(intptr_t)chromeTexture;
            glDeleteTextures(1, &texID);
        }

        LOG_INFO("Retro gauge textures cleaned up");
    }

} // namespace PrecisionTuner::Layers
