# Find ImGui package (from vcpkg)
find_package(imgui CONFIG REQUIRED)

# Main application
add_executable(precision-guitar-tuner
    PrecisionGuitarTuner.cpp
    PrecisionGuitarTunerApp.cpp
    Config.cpp
    TuningPresets.cpp
    Layers/AudioProcessingLayer.cpp
    Layers/TunerVisualizationLayer.cpp
    Layers/SettingsLayer.cpp
)

target_include_directories(precision-guitar-tuner PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Layers
    ${PROJECT_SOURCE_DIR}/external/stb
)

# Link kappa-core framework and ImGui
target_link_libraries(precision-guitar-tuner PRIVATE
    Kappa
    imgui::imgui
)

# Link lib-guitar-io and lib-guitar-dsp
if(TARGET guitar-io)
    target_link_libraries(precision-guitar-tuner PRIVATE guitar-io)
    message(STATUS "lib-guitar-io linked successfully")
endif()

if(TARGET guitar-dsp)
    target_link_libraries(precision-guitar-tuner PRIVATE guitar-dsp)
    message(STATUS "lib-guitar-dsp linked successfully")
endif()

# Platform-specific settings
if(WIN32)
    # Enable ASIO support (via RtAudio in lib-guitar-io)
    # TODO: Add when lib-guitar-io is ready

    # Keep console for now (skeleton app)
    # TODO: Change to WIN32_EXECUTABLE TRUE when GUI is ready
    set_target_properties(precision-guitar-tuner PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
elseif(APPLE)
    # CoreAudio frameworks (via RtAudio in lib-guitar-io)
    # TODO: Add when lib-guitar-io is ready
    # target_link_libraries(precision-guitar-tuner PRIVATE
    #     "-framework CoreAudio"
    #     "-framework AudioToolbox"
    #     "-framework CoreFoundation"
    # )

    # macOS bundle settings
    set_target_properties(precision-guitar-tuner PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.precisiontuner.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_BUNDLE_NAME "Precision Tuner"
    )
elseif(UNIX)
    # ALSA library for Linux (via RtAudio in lib-guitar-io)
    find_package(ALSA QUIET)
    if(ALSA_FOUND)
        # TODO: Add when lib-guitar-io is ready
        # target_link_libraries(precision-guitar-tuner PRIVATE ${ALSA_LIBRARIES})
        message(STATUS "Found ALSA: ${ALSA_LIBRARIES}")
    else()
        message(WARNING "ALSA not found. Audio I/O may not work on Linux.")
    endif()
endif()

# Apply compiler warnings
set_project_warnings(precision-guitar-tuner)

# Copy shader assets to build directory
add_custom_command(TARGET precision-guitar-tuner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/shaders
        $<TARGET_FILE_DIR:precision-guitar-tuner>/assets/shaders
    COMMENT "Copying shader assets"
)

# Installation
install(TARGETS precision-guitar-tuner
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install shader assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/shaders
    DESTINATION bin/assets
    FILES_MATCHING PATTERN "*.vert" PATTERN "*.frag"
)

# Install texture assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/textures
    DESTINATION bin/assets
    FILES_MATCHING PATTERN "*.png"
)

# Install LICENSE file (referenced in Packaging.cmake)
install(FILES ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION .
)
