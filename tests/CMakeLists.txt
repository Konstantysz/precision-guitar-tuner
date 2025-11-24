
# Test executable
add_executable(precision-tuner-tests
    ConfigTest.cpp
)

# Include directories
target_include_directories(precision-tuner-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Layers
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(precision-tuner-tests PRIVATE
    Kappa
    imgui::imgui
)

if(TARGET guitar-io)
    target_link_libraries(precision-tuner-tests PRIVATE guitar-io)
endif()

if(TARGET guitar-dsp)
    target_link_libraries(precision-tuner-tests PRIVATE guitar-dsp)
endif()

# Link against the main source files (since we don't have a separate library for the app logic yet)
# We need to compile the source files again for the test executable
target_sources(precision-tuner-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Config.cpp
    ${CMAKE_SOURCE_DIR}/src/TuningPresets.cpp
    ${CMAKE_SOURCE_DIR}/src/Layers/AudioProcessingLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/Layers/TunerVisualizationLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/Layers/SettingsLayer.cpp
)

# Add tests
add_test(NAME ConfigTest COMMAND precision-tuner-tests)
