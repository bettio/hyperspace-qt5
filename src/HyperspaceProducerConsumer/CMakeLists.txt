include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set(hyperspacerest_SRCS
    ConsumerAbstractAdaptor.cpp
    ProducerAbstractInterface.cpp
)

# Register here all the generated uppercase headers. The lowercase ones matching ones in this list will be installed as well
set(hyperspacerest_GENHEADERS
    ConsumerAbstractAdaptor
    ProducerAbstractInterface
)

hyperspace_generate_headers(hyperspacerest_HEADERS hyperspacerest_GENHEADERS)

if (ENABLE_HYPERSPACE_QT5_COVERAGE)
    add_library(ProducerConsumer STATIC ${hyperspacerest_SRCS})
    target_link_libraries(ProducerConsumer gcov)
else ()
    add_library(ProducerConsumer SHARED ${hyperspacerest_SRCS})
endif ()

set_target_properties(ProducerConsumer PROPERTIES
                                 OUTPUT_NAME HyperspaceQt5ProducerConsumer
                                 SOVERSION ${HYPERSPACE_QT5_ABI_VERSION}
                                 VERSION ${HYPERSPACE_QT5_VERSION}
                                 PUBLIC_HEADER "${hyperspacerest_HEADERS}")
target_link_libraries(ProducerConsumer Core Qt5::Core HemeraQt5SDK::Core)

install(TARGETS ProducerConsumer
        EXPORT  HyperspaceQt5ProducerConsumerTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/HyperspaceQt5/HyperspaceProducerConsumer"
        COMPONENT HyperspaceQt5)

# Export Hyperspace target
install(EXPORT HyperspaceQt5ProducerConsumerTargets NAMESPACE HyperspaceQt5:: DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5ProducerConsumer)

set(PRODUCERCONSUMER_INCLUDE_DIRS "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/HyperspaceQt5" "\${CMAKE_CURRENT_LIST_DIR}/${REL_INCLUDE_DIR}/HyperspaceQt5/HyperspaceProducerConsumer")
configure_file(HyperspaceQt5ProducerConsumerConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ProducerConsumerConfig.cmake @ONLY)
configure_file(${COMMONMODULES_CMAKE_DIR}/BasicFindPackageVersion.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ProducerConsumerConfigVersion.cmake @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ProducerConsumerConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ProducerConsumerConfigVersion.cmake
        DESTINATION ${INSTALL_CMAKE_DIR}/HyperspaceQt5ProducerConsumer COMPONENT HyperspaceQt5)

# Pkgconfig
configure_file(HyperspaceQt5ProducerConsumer.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ProducerConsumer.pc @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceQt5ProducerConsumer.pc
        DESTINATION ${INSTALL_PKGCONFIG_DIR} COMPONENT HyperspaceQt5)
