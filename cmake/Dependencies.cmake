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
