configure_file(HyperspacePaths.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/HyperspacePaths.cmake" @ONLY)

set(hyperspace_CMAKEMODULES
    HyperspaceMacros.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HyperspacePaths.cmake
)

# CMake modules
install(FILES ${hyperspace_CMAKEMODULES}
        DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5
        COMPONENT cmake)
