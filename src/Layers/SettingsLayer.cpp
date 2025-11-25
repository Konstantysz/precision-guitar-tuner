#include <Logger.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <AudioProcessingLayer.h>
#include <Config.h>
#include <SettingsLayer.h>
#include <TuningPresets.h>

namespace PrecisionTuner::Layers
{

    SettingsLayer::SettingsLayer(AudioProcessingLayer &audioLayer,
        TunerVisualizationLayer &tunerLayer,
        PrecisionTuner::Config &config)
        : audioLayer(audioLayer), tunerLayer(tunerLayer), config(config)
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
        // Only render if tuner layer indicates settings should be visible
        if (showSettings && tunerLayer.IsSettingsVisible())
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

                // Input device selection
                RenderInputDeviceSelector();

                ImGui::Separator();

                // Output device selection
                RenderOutputDeviceSelector();

                ImGui::Separator();

                // Audio feedback controls
                RenderAudioFeedbackControls();

                ImGui::Separator();

                // Reference pitch adjustment
                RenderReferencePitchSlider();

                ImGui::Separator();

                // Tuning mode selection
                RenderTuningModeSelector();
            }
            ImGui::End();

            ImGui::PopStyleVar();
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void SettingsLayer::RenderInputDeviceSelector()
    {
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Audio Input Device");

        // Refresh device list on first render
        static bool firstTime = true;
        if (firstTime)
        {
            availableInputDevices = audioLayer.GetAvailableInputDeviceInfo();
            firstTime = false;

            // Find current device in list
            uint32_t currentDeviceId = audioLayer.GetCurrentInputDeviceId();
            auto it = std::ranges::find_if(
                availableInputDevices, [currentDeviceId](const auto &device) { return device.id == currentDeviceId; });

            if (it != availableInputDevices.end())
            {
                selectedInputDeviceIndex = static_cast<int>(std::distance(availableInputDevices.begin(), it));
            }
        }

        if (ImGui::Button("Refresh Input Devices"))
        {
            availableInputDevices = audioLayer.GetAvailableInputDeviceInfo();
            LOG_INFO("Input device list refreshed - {} devices found", availableInputDevices.size());
        }

        // Device dropdown
        if (availableInputDevices.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No audio input devices found!");
            return;
        }

        // Create preview string for current selection
        const char *previewValue =
            selectedInputDeviceIndex >= 0 && selectedInputDeviceIndex < static_cast<int>(availableInputDevices.size())
                ? availableInputDevices[selectedInputDeviceIndex].name.c_str()
                : "Select device...";

        if (ImGui::BeginCombo("##InputDeviceCombo", previewValue))
        {
            for (int i = 0; i < static_cast<int>(availableInputDevices.size()); ++i)
            {
                const bool isSelected = (selectedInputDeviceIndex == i);
                if (ImGui::Selectable(availableInputDevices[i].name.c_str(), isSelected))
                {
                    if (selectedInputDeviceIndex != i)
                    {
                        selectedInputDeviceIndex = i;
                        uint32_t deviceId = availableInputDevices[i].id;

                        LOG_INFO("User selected input device: {}", availableInputDevices[i].name);

                        if (audioLayer.SwitchInputDevice(deviceId))
                        {
                            config.audio.deviceId = static_cast<int>(deviceId);
                            config.audio.deviceName = availableInputDevices[i].name;
                            LOG_INFO("Input device switched successfully");
                        }
                        else
                        {
                            LOG_ERROR("Failed to switch input device");
                        }
                    }
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Show device details
        if (selectedInputDeviceIndex >= 0 && selectedInputDeviceIndex < static_cast<int>(availableInputDevices.size()))
        {
            const auto &device = availableInputDevices[selectedInputDeviceIndex];
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

        // Tuning mode dropdown
        const char *tuningModes[] = {
            "Chromatic", "Standard (EADGBE)", "Drop D", "Drop C", "DADGAD", "Open G", "Open D"
        };
        int currentMode = static_cast<int>(config.tuning.mode);

        ImGui::PushItemWidth(200.0f);
        if (ImGui::Combo("##TuningMode", &currentMode, tuningModes, IM_ARRAYSIZE(tuningModes)))
        {
            config.tuning.mode = static_cast<TuningMode>(currentMode);
            LOG_INFO("Tuning mode changed to: {}", tuningModes[currentMode]);

            // Update polyphonic frequencies if polyphonic mode is active
            if (config.audio.enablePolyphonicMode)
            {
                auto preset = TuningPresets::GetPreset(config.tuning.mode, config.tuning.referencePitch);
                audioLayer.SetPolyphonicFrequencies(preset.targetFrequencies);
            }
        }
        ImGui::PopItemWidth();

        // Show helpful description based on selected mode
        if (config.tuning.mode == TuningMode::Chromatic)
        {
            ImGui::TextDisabled("Detects any note");
        }
        else
        {
            ImGui::TextDisabled("Shows target string indicator");
        }
    }

    void SettingsLayer::RenderOutputDeviceSelector()
    {
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Audio Output Device");

        // Refresh device list on first render
        static bool firstTimeOutput = true;
        if (firstTimeOutput)
        {
            availableOutputDevices = audioLayer.GetAvailableOutputDeviceInfo();
            firstTimeOutput = false;

            // Find current device in list
            uint32_t currentDeviceId = audioLayer.GetCurrentOutputDeviceId();
            auto it = std::ranges::find_if(
                availableOutputDevices, [currentDeviceId](const auto &device) { return device.id == currentDeviceId; });

            if (it != availableOutputDevices.end())
            {
                selectedOutputDeviceIndex = static_cast<int>(std::distance(availableOutputDevices.begin(), it));
            }
        }

        if (ImGui::Button("Refresh Output Devices"))
        {
            availableOutputDevices = audioLayer.GetAvailableOutputDeviceInfo();
            LOG_INFO("Output device list refreshed - {} devices found", availableOutputDevices.size());
        }

        // Device dropdown
        if (availableOutputDevices.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No audio output devices found!");
            return;
        }

        const char *previewValue =
            selectedOutputDeviceIndex >= 0
                    && selectedOutputDeviceIndex < static_cast<int>(availableOutputDevices.size())
                ? availableOutputDevices[selectedOutputDeviceIndex].name.c_str()
                : "Select device...";

        if (ImGui::BeginCombo("##OutputDeviceCombo", previewValue))
        {
            for (int i = 0; i < static_cast<int>(availableOutputDevices.size()); ++i)
            {
                const bool isSelected = (selectedOutputDeviceIndex == i);
                if (ImGui::Selectable(availableOutputDevices[i].name.c_str(), isSelected))
                {
                    if (selectedOutputDeviceIndex != i)
                    {
                        selectedOutputDeviceIndex = i;
                        uint32_t deviceId = availableOutputDevices[i].id;

                        LOG_INFO("User selected output device: {}", availableOutputDevices[i].name);

                        if (audioLayer.SwitchOutputDevice(deviceId))
                        {
                            config.audio.outputDeviceId = static_cast<int>(deviceId);
                            config.audio.outputDeviceName = availableOutputDevices[i].name;
                            LOG_INFO("Output device switched successfully");
                        }
                        else
                        {
                            LOG_ERROR("Failed to switch output device");
                        }
                    }
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // Show device details
        if (selectedOutputDeviceIndex >= 0
            && selectedOutputDeviceIndex < static_cast<int>(availableOutputDevices.size()))
        {
            const auto &device = availableOutputDevices[selectedOutputDeviceIndex];
            ImGui::TextDisabled("Channels: %u | ID: %u", device.maxOutputChannels, device.id);
        }
    }

    void SettingsLayer::RenderAudioFeedbackControls()
    {
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Audio Feedback");

        // Reference Tone
        bool enableReference = config.audio.enableReference;
        if (ImGui::Checkbox("Reference Tone", &enableReference))
        {
            config.audio.enableReference = enableReference;
            audioLayer.UpdateAudioFeedback(config.audio);
            LOG_INFO("Reference tone {}", enableReference ? "enabled" : "disabled");
        }

        if (config.audio.enableReference)
        {
            ImGui::Indent();

            // Reference frequency slider
            float referenceFrequency = config.audio.referenceFrequency;
            ImGui::PushItemWidth(150.0f);
            if (ImGui::SliderFloat("Frequency (Hz)", &referenceFrequency, 100.0f, 1000.0f, "%.1f Hz"))
            {
                config.audio.referenceFrequency = referenceFrequency;
                audioLayer.UpdateAudioFeedback(config.audio);
            }

            // Reference volume slider
            float referenceVolume = config.audio.referenceVolume;
            if (ImGui::SliderFloat("Volume##RefVol", &referenceVolume, 0.0f, 1.0f, "%.2f"))
            {
                config.audio.referenceVolume = referenceVolume;
                audioLayer.UpdateAudioFeedback(config.audio);
            }
            ImGui::PopItemWidth();
            ImGui::Unindent();
        }

        // Input Monitoring (Digital Amp)
        bool enableInputMonitoring = config.audio.enableInputMonitoring;
        if (ImGui::Checkbox("Input Monitoring (Digital Amp)", &enableInputMonitoring))
        {
            config.audio.enableInputMonitoring = enableInputMonitoring;
            audioLayer.UpdateAudioFeedback(config.audio);
            LOG_INFO("Input monitoring {}", enableInputMonitoring ? "enabled" : "disabled");
        }

        if (config.audio.enableInputMonitoring)
        {
            ImGui::Indent();
            float monitoringVolume = config.audio.monitoringVolume;
            ImGui::PushItemWidth(150.0f);
            if (ImGui::SliderFloat("Volume##MonVol", &monitoringVolume, 0.0f, 5.0f, "%.2f"))
            {
                config.audio.monitoringVolume = monitoringVolume;
                audioLayer.UpdateAudioFeedback(config.audio);
            }

            // Input gain slider
            float inputGain = config.audio.inputGain;
            if (ImGui::SliderFloat("Input Gain##InputGain", &inputGain, 0.0f, 5.0f, "%.2f"))
            {
                config.audio.inputGain = inputGain;
                audioLayer.UpdateAudioFeedback(config.audio);
            }

            // Input Level Meter
            float inputLevel = audioLayer.GetInputLevel();
            inputLevel = std::clamp(inputLevel, 0.0f, 1.0f);

            // Color based on level
            ImVec4 meterColor = ImVec4(0.2f, 0.9f, 0.3f, 1.0f); // Green
            if (inputLevel > 0.95f)
                meterColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Red (Clipping)
            else if (inputLevel > 0.75f)
                meterColor = ImVec4(0.9f, 0.8f, 0.2f, 1.0f); // Yellow

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, meterColor);
            ImGui::ProgressBar(inputLevel, ImVec2(0.0f, 0.0f), "");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextDisabled("Level");

            ImGui::PopItemWidth();
            ImGui::Unindent();
        }

        // In-Tune Beep
        bool beepEnabled = config.audio.enableBeep;
        if (ImGui::Checkbox("In-Tune Beep", &beepEnabled))
        {
            config.audio.enableBeep = beepEnabled;
            audioLayer.UpdateAudioFeedback(config.audio);
            LOG_INFO("In-tune beep {}", beepEnabled ? "enabled" : "disabled");
        }

        if (config.audio.enableBeep)
        {
            ImGui::Indent();
            float beepVolume = config.audio.beepVolume;
            ImGui::PushItemWidth(150.0f);
            if (ImGui::SliderFloat("Volume##BeepVol", &beepVolume, 0.0f, 1.0f, "%.2f"))
            {
                config.audio.beepVolume = beepVolume;
                audioLayer.UpdateAudioFeedback(config.audio);
            }
            ImGui::PopItemWidth();
            ImGui::TextDisabled("(Triggers when in-tune)");
            ImGui::Unindent();
        }

        ImGui::Separator();

        // Drone Mode - Continuous reference tone
        bool enableDroneMode = config.audio.enableDroneMode;
        if (ImGui::Checkbox("Drone Mode (Continuous Reference)", &enableDroneMode))
        {
            config.audio.enableDroneMode = enableDroneMode;
            // Disable polyphonic if drone is enabled
            if (enableDroneMode)
            {
                config.audio.enablePolyphonicMode = false;
            }
            audioLayer.UpdateAudioFeedback(config.audio);
            LOG_INFO("Drone mode {}", enableDroneMode ? "enabled" : "disabled");
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Plays a continuous reference tone while tuning");
        }

        // Polyphonic Mode - Play full chord
        bool enablePolyphonicMode = config.audio.enablePolyphonicMode;
        if (ImGui::Checkbox("Polyphonic Mode (Play Chord)", &enablePolyphonicMode))
        {
            config.audio.enablePolyphonicMode = enablePolyphonicMode;
            // Disable drone if polyphonic is enabled
            if (enablePolyphonicMode)
            {
                config.audio.enableDroneMode = false;

                // Set chord frequencies based on current tuning mode
                auto preset = TuningPresets::GetPreset(config.tuning.mode, config.tuning.referencePitch);
                audioLayer.SetPolyphonicFrequencies(preset.targetFrequencies);
            }
            audioLayer.UpdateAudioFeedback(config.audio);
            LOG_INFO("Polyphonic mode {}", enablePolyphonicMode ? "enabled" : "disabled");
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Plays all 6 string frequencies simultaneously for checking overall tuning");
        }
    }

} // namespace PrecisionTuner::Layers
