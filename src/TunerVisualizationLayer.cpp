#include "TunerVisualizationLayer.h"
#include <Logger.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Simple vertex shader
static const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fragColor;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    fragColor = aColor;
}
)";

// Simple fragment shader
static const char *fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(fragColor, 1.0);
}
)";

TunerVisualizationLayer::TunerVisualizationLayer(AudioProcessingLayer &audioLayer) : audioLayer(audioLayer)
{
    LOG_INFO("TunerVisualizationLayer - Initializing UI");
}

void TunerVisualizationLayer::InitializeOpenGL()
{
    LOG_INFO("TunerVisualizationLayer - Setting up OpenGL state");

    // Setup shaders
    SetupShaders();

    // Create VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Enable blending for smooth colors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth test (2D rendering)
    glDisable(GL_DEPTH_TEST);

    initialized = true;
}

void TunerVisualizationLayer::SetupShaders()
{
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        LOG_ERROR("Vertex shader compilation failed: {}", infoLog);
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        LOG_ERROR("Fragment shader compilation failed: {}", infoLog);
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed: {}", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_INFO("Shaders compiled and linked successfully");
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
    // Initialize OpenGL resources on first render
    if (!initialized)
    {
        InitializeOpenGL();
    }

    RenderBackground();

    // Always render UI elements (even without pitch data for testing)
    if (shaderProgram != 0)
    {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        RenderCentDeviationMeter();

        // Only render indicator if we have pitch data
        if (hasPitchData)
        {
            RenderTuningIndicator();
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }
}

void TunerVisualizationLayer::RenderBackground()
{
    // Clear screen to dark background
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void TunerVisualizationLayer::RenderCentDeviationMeter()
{
    // Render horizontal meter showing cent deviation from -50 to +50 cents
    const float meterY = 0.0f; // Center vertically

    // Draw meter background (dark gray)
    DrawFilledRect(
        -METER_WIDTH / 2.0f, meterY - METER_HEIGHT / 2.0f, METER_WIDTH, METER_HEIGHT, glm::vec3(0.2f, 0.2f, 0.22f));

    // Draw center line (white)
    DrawFilledRect(-0.002f, meterY - METER_HEIGHT / 2.0f, 0.004f, METER_HEIGHT, glm::vec3(1.0f, 1.0f, 1.0f));

    // Draw tick marks at ±10, ±20, ±30, ±40, ±50 cents
    for (int cent : { -50, -40, -30, -20, -10, 10, 20, 30, 40, 50 })
    {
        float x = (cent / 50.0f) * (METER_WIDTH / 2.0f);
        float tickHeight = (cent % 20 == 0) ? METER_HEIGHT * 0.6f : METER_HEIGHT * 0.4f;
        DrawFilledRect(x - 0.001f, meterY - tickHeight / 2.0f, 0.002f, tickHeight, glm::vec3(0.5f, 0.5f, 0.55f));
    }

    // Draw current cent deviation position (colored indicator)
    if (hasPitchData)
    {
        // Clamp cents to ±50 range for display
        float clampedCents = std::clamp(currentNote.cents, -50.0f, 50.0f);
        float indicatorX = (clampedCents / 50.0f) * (METER_WIDTH / 2.0f);

        glm::vec3 color = GetColorForCents(currentNote.cents);
        DrawFilledRect(indicatorX - 0.008f, meterY - METER_HEIGHT * 0.8f, 0.016f, METER_HEIGHT * 1.6f, color);
    }

    // Draw meter outline (white)
    DrawOutlineRect(-METER_WIDTH / 2.0f,
        meterY - METER_HEIGHT / 2.0f,
        METER_WIDTH,
        METER_HEIGHT,
        glm::vec3(0.6f, 0.6f, 0.65f),
        2.0f);
}

void TunerVisualizationLayer::RenderTuningIndicator()
{
    // Render circular indicator above the meter
    const float indicatorY = 0.3f;

    if (hasPitchData)
    {
        glm::vec3 color = GetColorForCents(currentNote.cents);
        DrawCircle(0.0f, indicatorY, INDICATOR_RADIUS, color, true);

        // Draw outline
        DrawCircle(0.0f, indicatorY, INDICATOR_RADIUS, glm::vec3(1.0f, 1.0f, 1.0f), false);
    }
}

glm::vec3 TunerVisualizationLayer::GetColorForCents(float cents)
{
    float absCents = std::abs(cents);

    if (absCents <= 3.0f)
    {
        // In tune - green
        return glm::vec3(0.2f, 0.9f, 0.3f);
    }
    else if (absCents <= 10.0f)
    {
        // Close - yellow-green blend
        float t = (absCents - 3.0f) / 7.0f;
        return glm::mix(glm::vec3(0.2f, 0.9f, 0.3f), glm::vec3(0.9f, 0.9f, 0.2f), t);
    }
    else if (absCents <= 25.0f)
    {
        // Getting far - yellow to orange
        float t = (absCents - 10.0f) / 15.0f;
        return glm::mix(glm::vec3(0.9f, 0.9f, 0.2f), glm::vec3(0.9f, 0.5f, 0.1f), t);
    }
    else
    {
        // Very out of tune - red
        return glm::vec3(0.9f, 0.2f, 0.2f);
    }
}

void TunerVisualizationLayer::DrawFilledRect(float x, float y, float width, float height, const glm::vec3 &color)
{
    // Create vertices for a filled rectangle (2 triangles)
    float vertices[] = {
        // positions        // colors
        x,
        y,
        color.r,
        color.g,
        color.b, // bottom-left
        x + width,
        y,
        color.r,
        color.g,
        color.b, // bottom-right
        x + width,
        y + height,
        color.r,
        color.g,
        color.b, // top-right

        x,
        y,
        color.r,
        color.g,
        color.b, // bottom-left
        x + width,
        y + height,
        color.r,
        color.g,
        color.b, // top-right
        x,
        y + height,
        color.r,
        color.g,
        color.b // top-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void TunerVisualizationLayer::DrawOutlineRect(float x,
    float y,
    float width,
    float height,
    const glm::vec3 &color,
    float lineWidth)
{
    // Create vertices for rectangle outline
    float vertices[] = {
        // positions        // colors
        x,
        y,
        color.r,
        color.g,
        color.b, // bottom-left
        x + width,
        y,
        color.r,
        color.g,
        color.b, // bottom-right
        x + width,
        y + height,
        color.r,
        color.g,
        color.b, // top-right
        x,
        y + height,
        color.r,
        color.g,
        color.b // top-left
    };

    glLineWidth(lineWidth);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void TunerVisualizationLayer::DrawCircle(float x, float y, float radius, const glm::vec3 &color, bool filled)
{
    const int segments = 32;
    std::vector<float> vertices;

    if (filled)
    {
        // Triangle fan: center + perimeter vertices
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);

        for (int i = 0; i <= segments; i++)
        {
            float angle = (i / static_cast<float>(segments)) * 2.0f * static_cast<float>(M_PI);
            float vx = x + radius * std::cos(angle);
            float vy = y + radius * std::sin(angle);
            vertices.push_back(vx);
            vertices.push_back(vy);
            vertices.push_back(color.r);
            vertices.push_back(color.g);
            vertices.push_back(color.b);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
    }
    else
    {
        // Line loop for outline
        for (int i = 0; i < segments; i++)
        {
            float angle = (i / static_cast<float>(segments)) * 2.0f * static_cast<float>(M_PI);
            float vx = x + radius * std::cos(angle);
            float vy = y + radius * std::sin(angle);
            vertices.push_back(vx);
            vertices.push_back(vy);
            vertices.push_back(color.r);
            vertices.push_back(color.g);
            vertices.push_back(color.b);
        }

        glLineWidth(3.0f);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINE_LOOP, 0, segments);
    }
}
