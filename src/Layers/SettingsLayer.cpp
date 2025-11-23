#include <SettingsLayer.h>
#include <AudioProcessingLayer.h>
#include <Config.h>
#include <Logger.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace PrecisionTuner::Layers
{

SettingsLayer::SettingsLayer(AudioProcessingLayer &audioLayer, PrecisionTuner::Config &config)
    : audioLayer(audioLayer), config(config)
{
    LOG_INFO("SettingsLayer - Initializing");
    InitializeImGui();
}

SettingsLayer::~SettingsLayer()
{
    if (imguiInitialized)
    {
        LOG_INFO("SettingsLayer - Shutting down ImGui");
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void SettingsLayer::InitializeImGui()
{
    // Get GLFW window from current context
    GLFWwindow *window = glfwGetCurrentContext();
    if (!window)
    {
        LOG_ERROR("Failed to initialize ImGui: No GLFW context");
        return;
    }

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Enable keyboard navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Initialize ImGui backends for GLFW + OpenGL3
    const char *glslVersion = "#version 430";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    imguiInitialized = true;
    LOG_INFO("ImGui initialized successfully");
}

void SettingsLayer::OnUpdate([[maybe_unused]] float deltaTime)
{
    if (!imguiInitialized)
    {
        return;
    }

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void SettingsLayer::OnRender()
{
    if (!imguiInitialized)
    {
        return;
    }

    // Create settings window (positioned in bottom right corner)
    if (showSettings)
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 windowSize(450, 350);
        ImVec2 windowPos(viewport->Size.x - windowSize.x - 20, viewport->Size.y - windowSize.y - 20);

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

        // Make settings window semi-transparent
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.95f);

        if (ImGui::Begin("Tuner Settings", &showSettings, ImGuiWindowFlags_NoCollapse))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Precision Guitar Tuner");
            ImGui::Separator();

            // Audio device selection
            RenderDeviceSelector();

            ImGui::Separator();

            // Reference pitch adjustment
            RenderReferencePitchSlider();

            ImGui::Separator();

            // Tuning mode selection (placeholder for future)
            RenderTuningModeSelector();
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SettingsLayer::RenderDeviceSelector()
{
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Audio Input Device");

    // Refresh device list on first render or on button click
    static bool firstTime = true;
    if (firstTime)
    {
        availableDevices = audioLayer.GetAvailableDeviceInfo();
        firstTime = false;

        // Find current device in list
        uint32_t currentDeviceId = audioLayer.GetCurrentDeviceId();
        auto it = std::ranges::find_if(availableDevices,
            [currentDeviceId](const auto &device) { return device.id == currentDeviceId; });

        if (it != availableDevices.end())
        {
            selectedDeviceIndex = static_cast<int>(std::distance(availableDevices.begin(), it));
        }
    }

    if (ImGui::Button("Refresh Devices"))
    {
        availableDevices = audioLayer.GetAvailableDeviceInfo();
        LOG_INFO("Device list refreshed - {} devices found", availableDevices.size());
    }

    ImGui::SameLine();
    ImGui::TextDisabled("(Click to rescan)");

    // Device dropdown
    if (availableDevices.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No audio devices found!");
        return;
    }

    // Create preview string for current selection
    const char *previewValue = selectedDeviceIndex >= 0 && selectedDeviceIndex < static_cast<int>(availableDevices.size())
                                    ? availableDevices[selectedDeviceIndex].name.c_str()
                                    : "Select device...";

    if (ImGui::BeginCombo("##DeviceCombo", previewValue))
    {
        for (int i = 0; i < static_cast<int>(availableDevices.size()); ++i)
        {
            const bool isSelected = (selectedDeviceIndex == i);
            if (ImGui::Selectable(availableDevices[i].name.c_str(), isSelected))
            {
                if (selectedDeviceIndex != i)
                {
                    selectedDeviceIndex = i;
                    uint32_t deviceId = availableDevices[i].id;

                    LOG_INFO("User selected device: {}", availableDevices[i].name);

                    // Switch to new device
                    if (audioLayer.SwitchDevice(deviceId))
                    {
                        // Update config for persistence
                        config.audio.deviceId = static_cast<int>(deviceId);
                        config.audio.deviceName = availableDevices[i].name;
                        LOG_INFO("Device switched successfully and config updated");
                    }
                    else
                    {
                        LOG_ERROR("Failed to switch device");
                    }
                }
            }

            // Set the initial focus when opening the combo
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Show device details
    if (selectedDeviceIndex >= 0 && selectedDeviceIndex < static_cast<int>(availableDevices.size()))
    {
        const auto &device = availableDevices[selectedDeviceIndex];
        ImGui::TextDisabled("Channels: %u | ID: %u", device.maxInputChannels, device.id);
    }
}

void SettingsLayer::RenderReferencePitchSlider()
{
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Reference Pitch (A4)");

    // Slider for reference pitch (430-450 Hz)
    float referencePitch = config.tuning.referencePitch;

    ImGui::PushItemWidth(200.0f);
    if (ImGui::SliderFloat("##RefPitch", &referencePitch, 430.0f, 450.0f, "%.1f Hz"))
    {
        config.tuning.referencePitch = referencePitch;
        LOG_INFO("Reference pitch updated: {:.1f} Hz", referencePitch);
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Reset##RefPitchReset"))
    {
        config.tuning.referencePitch = 440.0f;
        LOG_INFO("Reference pitch reset to 440.0 Hz");
    }

    ImGui::TextDisabled("Standard: 440 Hz | Common range: 430-450 Hz");
}

void SettingsLayer::RenderTuningModeSelector()
{
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Tuning Mode");

    // Tuning mode dropdown (placeholder for now)
    const char *tuningModes[] = { "Chromatic", "Standard (EADGBE)", "Drop D", "Drop C", "DADGAD", "Open G", "Open D" };
    int currentMode = static_cast<int>(config.tuning.mode);

    ImGui::PushItemWidth(200.0f);
    if (ImGui::Combo("##TuningMode", &currentMode, tuningModes, IM_ARRAYSIZE(tuningModes)))
    {
        config.tuning.mode = static_cast<TuningMode>(currentMode);
        LOG_INFO("Tuning mode changed to: {}", tuningModes[currentMode]);
    }
    ImGui::PopItemWidth();

    ImGui::TextDisabled("Note: Only Chromatic mode is currently implemented");
}

} // namespace PrecisionTuner::Layers
