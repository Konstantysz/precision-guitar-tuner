cmake_minimum_required(VERSION 3.20)

# Set vcpkg triplet before project()
if(NOT DEFINED VCPKG_TARGET_TRIPLET AND WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
endif()

project(PrecisionGuitarTuner VERSION 0.0.1 LANGUAGES CXX)

# Coverage option (inherits from kappa-core if enabled there)
option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)

if(ENABLE_COVERAGE)
    # Coverage flags will be inherited from kappa-core submodule
    message(STATUS "Code coverage enabled for precision-guitar-tuner")
endif()

# Include project modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(StandardSettings)
include(CompilerWarnings)
include(Dependencies)

# Add source directory
add_subdirectory(src)

# Testing
enable_testing()
add_subdirectory(tests)

# Packaging
include(Packaging)

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
