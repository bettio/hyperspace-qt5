include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set(hyperspaceclient_SRCS
    hyperspacecapabilitywatcher.cpp
    hyperspaceglobaldiscoverywatcher.cpp
)

# Register here all the generated uppercase headers. The lowercase ones matching ones in this list will be installed as well
set(hyperspaceclient_GENHEADERS
    CapabilityWatcher
)

hyperspace_internal_generate_headers(hyperspaceclient_HEADERS hyperspaceclient_GENHEADERS)

if (ENABLE_HYPERSPACE_QT5_COVERAGE)
    add_library(Client STATIC ${hyperspaceclient_SRCS})
    target_link_libraries(Client gcov)
else ()
    add_library(Client SHARED ${hyperspaceclient_SRCS})
endif ()

set_target_properties(Client PROPERTIES
                                 OUTPUT_NAME HyperspaceQt5Client
                                 SOVERSION ${HYPERSPACE_QT5_ABI_VERSION}
                                 VERSION ${HYPERSPACE_QT5_VERSION}
                                 PUBLIC_HEADER "${hyperspaceclient_HEADERS}")
qt5_use_modules(Client Core Network)
hemera_qt5_sdk_use_modules(Client Core)
target_link_libraries(Client Core)

install(TARGETS Client
        EXPORT  HyperspaceQt5ClientTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/HyperspaceQt5/HyperspaceClient"
        COMPONENT HyperspaceQt5)

# Export Hyperspace target
install(EXPORT HyperspaceQt5ClientTargets NAMESPACE HyperspaceQt5:: DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5Client)

set(CLIENT_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/HyperspaceQt5" "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/HyperspaceQt5/HyperspaceClient")
configure_file(HyperspaceQt5ClientConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ClientConfig.cmake @ONLY)
configure_file(${COMMONMODULES_CMAKE_DIR}/BasicFindPackageVersion.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ClientConfigVersion.cmake @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ClientConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ClientConfigVersion.cmake
        DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5Client COMPONENT HyperspaceQt5)

# Pkgconfig
configure_file(HyperspaceQt5Client.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5Client.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5Client.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR} COMPONENT HyperspaceQt5)
