include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set(hyperspacecore_SRCS
    AbstractWaveTarget.cpp
    BSONDocument.cpp
    BSONSerializer.cpp
    BSONStreamReader.cpp
    Fluctuation.cpp
    Gate.cpp
    Rebound.cpp
    Socket.cpp
    Wave.cpp
    Waveguide.cpp
)

# Register here all the generated uppercase headers. The lowercase ones matching ones in this list will be installed as well
set(hyperspacecore_GENHEADERS
    AbstractWaveTarget
    BSONDocument
    BSONSerializer
    BSONStreamReader
    Fluctuation
    Gate
    Global
    Rebound
    Socket
    Wave
    Waveguide
)

hyperspace_generate_headers(hyperspacecore_HEADERS hyperspacecore_GENHEADERS)

if (ENABLE_HYPERSPACE_QT5_COVERAGE)
    add_library(Core STATIC ${hyperspacecore_SRCS})
    target_link_libraries(Core gcov)
else ()
    add_library(Core SHARED ${hyperspacecore_SRCS})
endif ()

set_target_properties(Core PROPERTIES
                           OUTPUT_NAME HyperspaceQt5Core
                           SOVERSION ${HYPERSPACE_QT5_ABI_VERSION}
                           VERSION ${HYPERSPACE_QT5_VERSION}
                           PUBLIC_HEADER "${hyperspacecore_HEADERS}")
target_link_libraries(Core Qt5::Core HemeraQt5SDK::Core)

install(TARGETS Core
        EXPORT  HyperspaceQt5CoreTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/HyperspaceQt5/HyperspaceCore"
        COMPONENT HyperspaceQt5)

# Export Hyperspace target
install(EXPORT HyperspaceQt5CoreTargets NAMESPACE HyperspaceQt5:: DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5Core)

set(CORE_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/HyperspaceQt5" "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/HyperspaceQt5/HyperspaceCore")
configure_file(HyperspaceQt5CoreConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5CoreConfig.cmake @ONLY)
configure_file(${COMMONMODULES_CMAKE_DIR}/BasicFindPackageVersion.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5CoreConfigVersion.cmake @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5CoreConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5CoreConfigVersion.cmake
        DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5Core COMPONENT HyperspaceQt5)

# Pkgconfig
configure_file(HyperspaceQt5Core.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5Core.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5Core.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR} COMPONENT HyperspaceQt5)
