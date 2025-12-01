# CPack Packaging Configuration
set(CPACK_PACKAGE_NAME "PrecisionGuitarTuner")
set(CPACK_PACKAGE_VENDOR "PrecisionTuner")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional-grade guitar tuner")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "PrecisionGuitarTuner")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "ZIP") # Simple ZIP for now, NSIS requires installation
    set(CPACK_NSIS_DISPLAY_NAME "Precision Guitar Tuner")
    set(CPACK_NSIS_PACKAGE_NAME "Precision Guitar Tuner")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Precision Tuner")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ") # Tarball for Linux, AppImage handled by CI
endif()

include(CPack)
