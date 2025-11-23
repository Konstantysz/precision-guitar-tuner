cmake_minimum_required(VERSION 3.20)

# Set vcpkg triplet before project()
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
endif()

project(PrecisionGuitarTuner VERSION 0.0.1 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform detection macros
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Git submodule initialization
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Initializing git submodules...")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}")
        endif()
    endif()
endif()

# Verify kappa-core submodule exists
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/kappa-core/CMakeLists.txt")
    message(FATAL_ERROR "kappa-core submodule not found. Run: git submodule update --init --recursive")
endif()

# Add kappa-core framework
add_subdirectory(external/kappa-core)

# Verify lib-guitar-io exists
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/lib-guitar-io/CMakeLists.txt")
    message(WARNING "lib-guitar-io not found. Audio I/O will not be available.")
else()
    add_subdirectory(external/lib-guitar-io)
endif()

# Verify lib-guitar-dsp exists
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/lib-guitar-dsp/CMakeLists.txt")
    message(WARNING "lib-guitar-dsp not found. DSP algorithms will not be available.")
else()
    add_subdirectory(external/lib-guitar-dsp)
endif()

# Main application
add_executable(precision-guitar-tuner
    src/main.cpp
    src/AudioProcessingLayer.cpp
    src/TunerVisualizationLayer.cpp
    src/FontRenderer.cpp
)

target_include_directories(precision-guitar-tuner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

# Link kappa-core framework
target_link_libraries(precision-guitar-tuner PRIVATE
    Kappa
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

# Compiler warnings
if(MSVC)
    target_compile_options(precision-guitar-tuner PRIVATE /WX)
else()
    target_compile_options(precision-guitar-tuner PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter -Werror
    )
endif()

# Installation
install(TARGETS precision-guitar-tuner
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Testing (TODO: add when tests are created)
# enable_testing()
# add_subdirectory(tests)

# Print configuration summary
message(STATUS "")
message(STATUS "============================================")
message(STATUS "  Precision Guitar Tuner v${PROJECT_VERSION}")
message(STATUS "============================================")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "============================================")
message(STATUS "")
