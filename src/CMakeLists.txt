include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceCore
                    ${CMAKE_CURRENT_SOURCE_DIR}/HyperspaceCore
                    ${CMAKE_CURRENT_BINARY_DIR}/HyperspaceProducerConsumer
                    ${CMAKE_CURRENT_SOURCE_DIR}/HyperspaceProducerConsumer
)

# We are building the library
add_definitions(-DBUILDING_HYPERSPACE)

# Config
configure_file(hyperspaceconfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/hyperspaceconfig.h" @ONLY)

# Useful macros
macro(hyperspace_internal_generate_headers_prefix _prefix _output _list)
    # This loop generates the headers
    foreach (f ${${_list}})
        string(TOLOWER ${f} gtmp)
        set(gen-header ${_prefix}${gtmp}.h)

        if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${gen-header}")
            message(FATAL_ERROR "The file ${f} does not have a corresponding ${gen-header} file. Aborting.")
        endif ()

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${f} "#include \"${gen-header}\"\n")

        list(APPEND ${_output} ${CMAKE_CURRENT_BINARY_DIR}/${f} ${CMAKE_CURRENT_SOURCE_DIR}/${gen-header})
    endforeach (f)
endmacro()

macro(hyperspace_generate_headers _output _list)
    # This loop generates the headers
    foreach (f ${${_list}})
        set(gen-header ${f}.h)

        if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${gen-header}")
            message(FATAL_ERROR "The file ${f} does not have a corresponding ${gen-header} file. Aborting.")
        endif ()

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${f} "#include \"${gen-header}\"\n")

        list(APPEND ${_output} ${CMAKE_CURRENT_BINARY_DIR}/${f} ${CMAKE_CURRENT_SOURCE_DIR}/${gen-header})
    endforeach (f)
endmacro()


macro(hyperspace_internal_generate_headers _output _list)
    hyperspace_internal_generate_headers_prefix("hyperspace" ${_output} ${_list})
endmacro()

# Modules
add_subdirectory(HyperspaceClient)
add_subdirectory(HyperspaceCore)
add_subdirectory(HyperspaceProducerConsumer)
