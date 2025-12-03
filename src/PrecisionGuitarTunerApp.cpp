#include "PrecisionGuitarTunerApp.h"
#include <algorithm>
#include <TuningPresets.h>

PrecisionGuitarTunerApp::PrecisionGuitarTunerApp()
    : Application(CreateApplicationSpecification(PrecisionTuner::Config::Load())),
      config(PrecisionTuner::Config::Load()), audioLayer(nullptr), tunerLayer(nullptr), settingsLayer(nullptr)
{
    LOG_INFO("Precision Guitar Tuner initialized");

    InitializeImGui();

    PrecisionTuner::Layers::AudioProcessingLayerConfig audioLayerConfig;
    audioLayerConfig.sampleRate = static_cast<uint32_t>(config.audio.sampleRate);
    audioLayerConfig.bufferSize = static_cast<uint32_t>(config.audio.bufferSize);

    PushLayer<PrecisionTuner::Layers::AudioProcessingLayer>(audioLayerConfig);

    audioLayer = dynamic_cast<PrecisionTuner::Layers::AudioProcessingLayer *>(GetLayers().back().get());

    if (!audioLayer)
    {
        LOG_ERROR("CRITICAL: Failed to initialize AudioProcessingLayer");
        throw std::runtime_error("Failed to initialize audio system");
    }

    audioLayer->UpdateAudioFeedback(config.audio);

    PushLayer<PrecisionTuner::Layers::TunerVisualizationLayer>(*audioLayer, config);

    tunerLayer = dynamic_cast<PrecisionTuner::Layers::TunerVisualizationLayer *>(GetLayers().back().get());
    if (!tunerLayer)
    {
        LOG_ERROR("CRITICAL: Failed to initialize TunerVisualizationLayer");
        throw std::runtime_error("Failed to initialize visualization system");
    }

    PushLayer<PrecisionTuner::Layers::SettingsLayer>(*audioLayer, *tunerLayer, config);

    settingsLayer = dynamic_cast<PrecisionTuner::Layers::SettingsLayer *>(GetLayers().back().get());
    if (!settingsLayer)
    {
        LOG_ERROR("CRITICAL: Failed to initialize SettingsLayer");
        throw std::runtime_error("Failed to initialize settings system");
    }

    LOG_INFO("All layers initialized");
}

PrecisionGuitarTunerApp::~PrecisionGuitarTunerApp()
{
    LOG_INFO("Precision Tuner shutting down");

    ShutdownImGui();

    GLFWwindow *window = glfwGetCurrentContext();
    if (window)
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);

        // Apply minimum and maximum constraints
        config.window.width =
            std::clamp(width, PrecisionTuner::WindowConfig::MIN_WIDTH, PrecisionTuner::WindowConfig::MAX_WIDTH);
        config.window.height =
            std::clamp(height, PrecisionTuner::WindowConfig::MIN_HEIGHT, PrecisionTuner::WindowConfig::MAX_HEIGHT);
    }

    // Save configuration
    if (config.Save())
    {
        LOG_INFO("Configuration saved successfully");
    }
    else
    {
        LOG_ERROR("Failed to save configuration");
    }
}

void PrecisionGuitarTunerApp::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    HandleKeyboardInput();
}

void PrecisionGuitarTunerApp::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void PrecisionGuitarTunerApp::HandleKeyboardInput()
{
    // Skip keyboard shortcuts if typing in a text field
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantTextInput)
    {
        return;
    }

    // Audio feedback toggles
    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        config.audio.enableInputMonitoring = !config.audio.enableInputMonitoring;
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("Input monitoring {}", config.audio.enableInputMonitoring ? "enabled" : "disabled");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_D))
    {
        config.audio.enableDroneMode = !config.audio.enableDroneMode;
        if (config.audio.enableDroneMode)
        {
            config.audio.enablePolyphonicMode = false;
        }
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("Drone mode {}", config.audio.enableDroneMode ? "enabled" : "disabled");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_P))
    {
        config.audio.enablePolyphonicMode = !config.audio.enablePolyphonicMode;
        if (config.audio.enablePolyphonicMode)
        {
            config.audio.enableDroneMode = false;
            auto preset = PrecisionTuner::TuningPresets::GetPreset(config.tuning.mode, config.tuning.referencePitch);
            audioLayer->SetPolyphonicFrequencies(preset.targetFrequencies);
        }
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("Polyphonic mode {}", config.audio.enablePolyphonicMode ? "enabled" : "disabled");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_R))
    {
        config.audio.enableReference = !config.audio.enableReference;
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("Reference tone {}", config.audio.enableReference ? "enabled" : "disabled");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_B))
    {
        config.audio.enableBeep = !config.audio.enableBeep;
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("In-tune beep {}", config.audio.enableBeep ? "enabled" : "disabled");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_M))
    {
        config.audio.enableInputMonitoring = false;
        config.audio.enableDroneMode = false;
        config.audio.enablePolyphonicMode = false;
        config.audio.enableReference = false;
        config.audio.enableBeep = false;
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("All audio feedback muted");
    }

    // Input gain adjustment
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        config.audio.inputGain = std::min(config.audio.inputGain + 0.1f, 2.0f);
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("Input gain increased to {:.1f}", config.audio.inputGain);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        config.audio.inputGain = std::max(config.audio.inputGain - 0.1f, 0.5f);
        audioLayer->UpdateAudioFeedback(config.audio);
        LOG_INFO("Input gain decreased to {:.1f}", config.audio.inputGain);
    }

    // Settings panel toggle
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Comma))
    {
        tunerLayer->SetSettingsVisible(true);
        LOG_INFO("Settings opened");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        tunerLayer->SetSettingsVisible(false);
        LOG_INFO("Settings closed");
    }

    // Fullscreen toggle
    if (ImGui::IsKeyPressed(ImGuiKey_F11))
    {
        GLFWwindow *window = glfwGetCurrentContext();
        if (window)
        {
            GLFWmonitor *monitor = glfwGetWindowMonitor(window);
            if (monitor == nullptr)
            {
                GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
                glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                LOG_INFO("Fullscreen enabled");
            }
            else
            {
                glfwSetWindowMonitor(window, nullptr, 100, 100, config.window.width, config.window.height, 0);
                LOG_INFO("Fullscreen disabled");
            }
        }
    }

    // F1 for keyboard shortcuts overlay
    if (ImGui::IsKeyPressed(ImGuiKey_F1))
    {
        settingsLayer->ToggleKeyboardShortcuts();
    }
}

void PrecisionGuitarTunerApp::InitializeImGui()
{
    GLFWwindow *window = glfwGetCurrentContext();
    if (!window)
    {
        LOG_ERROR("Failed to initialize ImGui: No GLFW context");
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    const char *glslVersion = "#version 430";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    LOG_INFO("ImGui initialized successfully");
}

void PrecisionGuitarTunerApp::ShutdownImGui()
{
    LOG_INFO("Shutting down ImGui");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

Kappa::ApplicationSpecification PrecisionGuitarTunerApp::CreateApplicationSpecification(
    const PrecisionTuner::Config &config)
{
    return Kappa::ApplicationSpecification{ .name = "Precision Guitar Tuner",
        .windowSpecification = {
            .title = "Precision Guitar Tuner v0.0.3-alpha",
            .width = static_cast<unsigned int>(config.window.width),
            .height = static_cast<unsigned int>(config.window.height),
            .isResizable = true // Enable resizing for Phase 3 responsive layout
        } };
}
