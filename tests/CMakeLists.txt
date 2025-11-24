
# Config Test executable
add_executable(config-test
    ConfigTest.cpp
)

# Include directories for config test
target_include_directories(config-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Layers
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries for config test
target_link_libraries(config-test PRIVATE
    Kappa
    imgui::imgui
)

if(TARGET guitar-io)
    target_link_libraries(config-test PRIVATE guitar-io)
endif()

if(TARGET guitar-dsp)
    target_link_libraries(config-test PRIVATE guitar-dsp)
endif()

# Link source files for config test
target_sources(config-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Config.cpp
)

# TuningPresets Test executable
add_executable(tuning-presets-test
    TestTuningPresets.cpp
)

# Include directories for tuning presets test
target_include_directories(tuning-presets-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Layers
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries for tuning presets test
target_link_libraries(tuning-presets-test PRIVATE
    Kappa
    imgui::imgui
)

if(TARGET guitar-io)
    target_link_libraries(tuning-presets-test PRIVATE guitar-io)
endif()

if(TARGET guitar-dsp)
    target_link_libraries(tuning-presets-test PRIVATE guitar-dsp)
endif()

# Link source files for tuning presets test
target_sources(tuning-presets-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Config.cpp
    ${CMAKE_SOURCE_DIR}/src/TuningPresets.cpp
)

# Add tests
add_test(NAME ConfigTest COMMAND config-test)
add_test(NAME TuningPresetsTest COMMAND tuning-presets-test)

