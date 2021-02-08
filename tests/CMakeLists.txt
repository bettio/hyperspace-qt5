include_directories(${CMAKE_BINARY_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_BINARY_DIR}/src/HyperspaceCore
                    ${CMAKE_SOURCE_DIR}/src/HyperspaceCore
                    ${CMAKE_BINARY_DIR}/src/HyperspaceProducerConsumer
                    ${CMAKE_SOURCE_DIR}/src/HyperspaceProducerConsumer
)

add_subdirectory(lib)

# Test librarieset(TestLibraries Core ProducerConsumer HyperspaceTestLib)
set(TestLibraries Core ProducerConsumer HyperspaceTestLib)

hemera_add_unit_test(BSONBasics bson-basics ${TestLibraries})

# # KeyValueJsonSerializer
# set(KeyValueJsonSerializer_SRCS lib/testrestpropertyresource.cpp keyvaluejsonserializertest.cpp)
# # qt5_automoc(${KeyValueJsonSerializer_SRCS})
# add_executable(KeyValueJsonSerializerTest ${KeyValueJsonSerializer_SRCS})
# target_link_libraries(KeyValueJsonSerializerTest ${TestLibraries})
# add_test(NAME KeyValueJsonSerializerTest COMMAND KeyValueJsonSerializerTest)

# set(KeyValueJsonSerializer_SRCS lib/testrestpropertyresource.cpp keyvaluejsonserializertest.cpp)
# # qt5_automoc(${KeyValueJsonSerializer_SRCS})
# add_executable(KeyValueJsonSerializerTest ${KeyValueJsonSerializer_SRCS})
# target_link_libraries(KeyValueJsonSerializerTest ${TestLibraries})
# add_test(NAME KeyValueJsonSerializerTest COMMAND KeyValueJsonSerializerTest)

# This should always be built and installed for packaging reasons
