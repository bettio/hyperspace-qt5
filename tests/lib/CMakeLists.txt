set(hyperspacetestlib_SRCS
    fakehyperdrive.cpp
)

add_library(HyperspaceTestLib STATIC ${hyperspacetestlib_SRCS})
if (ENABLE_HYPERSPACE_QT5_COVERAGE)
    target_link_libraries(HyperspaceTestLib gcov)
endif ()

target_link_libraries(HyperspaceTestLib Core ProducerConsumer Qt5::Core Qt5::Test HemeraQt5SDK::Core HemeraQt5SDK::Test)
