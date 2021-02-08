include_directories(${CMAKE_BINARY_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BINARY_DIR}/src/HyperspaceCore
                    ${CMAKE_SOURCE_DIR}/src/HyperspaceCore
)

set(hyperspace2cpp_SRCS
    main.cpp
    hyperspace2cpp.cpp
)

# final libraries
add_executable(hyperspace2cpp ${hyperspace2cpp_SRCS})
target_link_libraries(hyperspace2cpp Qt5::Core Qt5::Qml HemeraQt5SDK::Core HemeraQt5SDK::Generators)

if (ENABLE_HYPERSPACE_QT5_COVERAGE)
    target_link_libraries(hyperspace2cpp gcov)
endif ()

# Install phase
install(TARGETS hyperspace2cpp
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_BIN_DIR}" COMPONENT shlib
        COMPONENT Tools)

install(FILES hyperspaceconsumerinterface.h.in hyperspaceconsumerinterface.cpp.in
              hyperspaceproducerinterface.h.in hyperspaceproducerinterface.cpp.in
              hyperspaceaggregateproducerinterface.h.in hyperspaceaggregateproducerinterface.cpp.in
        DESTINATION ${INSTALL_DATA_DIR})

