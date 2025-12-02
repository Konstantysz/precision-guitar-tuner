
# Find Google Test (provided by vcpkg via kappa-core or directly)
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
include(GoogleTest)

# Config Test executable
add_executable(test-config
    TestConfig.cpp
)

# Include directories for config test
target_include_directories(test-config PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Layers
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/kappa-core/include
)

# Link libraries for config test
target_link_libraries(test-config PRIVATE
    spdlog::spdlog
    GTest::gtest
    GTest::gtest_main
)

if(TARGET guitar-io)
    target_link_libraries(test-config PRIVATE guitar-io)
endif()

if(TARGET guitar-dsp)
    target_link_libraries(test-config PRIVATE guitar-dsp)
endif()

# Link source files for config test
target_sources(test-config PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Config.cpp
)

# TuningPresets Test executable
add_executable(test-tuning-presets
    TestTuningPresets.cpp
)

# Include directories for tuning presets test
target_include_directories(test-tuning-presets PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Layers
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/kappa-core/include
)

# Link libraries for tuning presets test
target_link_libraries(test-tuning-presets PRIVATE
    spdlog::spdlog
    GTest::gtest
    GTest::gtest_main
)

if(TARGET guitar-io)
    target_link_libraries(test-tuning-presets PRIVATE guitar-io)
endif()

if(TARGET guitar-dsp)
    target_link_libraries(test-tuning-presets PRIVATE guitar-dsp)
endif()

# Link source files for tuning presets test
target_sources(test-tuning-presets PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Config.cpp
    ${CMAKE_SOURCE_DIR}/src/TuningPresets.cpp
)

# Register tests with CTest using GoogleTest module
gtest_discover_tests(test-config)
gtest_discover_tests(test-tuning-presets)

# Pitch Stabilizer Test executable
add_executable(test-pitch-stabilizer
    TestPitchStabilizer.cpp
)

# Include directories for pitch stabilizer test
target_include_directories(test-pitch-stabilizer PRIVATE
    ${CMAKE_SOURCE_DIR}/external/lib-guitar-dsp/include
)

# Link libraries for pitch stabilizer test
target_link_libraries(test-pitch-stabilizer PRIVATE
    guitar-dsp
    GTest::gtest
    GTest::gtest_main
)

# Register pitch stabilizer tests
gtest_discover_tests(test-pitch-stabilizer)

# AudioProcessingLayer Test executable
add_executable(test-audio-layer
    TestAudioProcessingLayer.cpp
)

# Include directories for audio layer test
target_include_directories(test-audio-layer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Layers
    ${CMAKE_SOURCE_DIR}/external/lib-guitar-io/include
    ${CMAKE_SOURCE_DIR}/external/lib-guitar-dsp/include
    ${CMAKE_SOURCE_DIR}/external/kappa-core/include
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries for audio layer test
target_link_libraries(test-audio-layer PRIVATE
    guitar-io
    guitar-dsp
    spdlog::spdlog
    GTest::gtest
    GTest::gtest_main
)

# Link source files for audio layer test
target_sources(test-audio-layer PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Layers/AudioProcessingLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/Config.cpp
    ${CMAKE_SOURCE_DIR}/tests/mocks/MockAudioDevice.cpp
)

# Register audio layer tests
gtest_discover_tests(test-audio-layer)
