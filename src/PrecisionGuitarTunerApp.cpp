#include "PrecisionGuitarTunerApp.h"

PrecisionGuitarTunerApp::PrecisionGuitarTunerApp()
    : Application(CreateApplicationSpecification(PrecisionTuner::Config::Load())),
      config(PrecisionTuner::Config::Load()), audioLayer(nullptr)
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

    auto *tunerLayer = dynamic_cast<PrecisionTuner::Layers::TunerVisualizationLayer *>(GetLayers().back().get());
    if (!tunerLayer)
    {
        LOG_ERROR("CRITICAL: Failed to initialize TunerVisualizationLayer");
        throw std::runtime_error("Failed to initialize visualization system");
    }

    PushLayer<PrecisionTuner::Layers::SettingsLayer>(*audioLayer, *tunerLayer, config);

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
}

void PrecisionGuitarTunerApp::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
