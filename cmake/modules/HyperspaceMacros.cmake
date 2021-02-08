function(hyperspace_add_qt5_consumer _sources _interface_file _include _parentClass) # _optionalBasename # _optionalClassName
    get_filename_component(_infile ${_interface_file} ABSOLUTE)
    if (NOT EXISTS ${_infile})
        message(FATAL_ERROR "Cannot find file ${_infile}. Aborting.")
    endif ()

    set(_optionalBasename "${ARGV4}")
    if(_optionalBasename)
        set(_basename ${_optionalBasename} )
    else()
        string(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.json" "\\2consumer" _basename ${_infile})
        string(TOLOWER ${_basename} _basename)
    endif()

    set(_optionalClassName "${ARGV5}")
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h")
    set(_impl   "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp")
    set(_moc    "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc")

    if(_optionalClassName)
        add_custom_command(OUTPUT "${_impl}" "${_header}"
          COMMAND ${HYPERSPACE_TOOLS_DIR}/hyperspace2cpp -a ${_basename} -c ${_optionalClassName} -i ${_include} -l ${_parentClass} ${_infile}
          DEPENDS ${_infile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating Hyperspace Consumer ${_basename}.h, ${_basename}.cpp..." VERBATIM
        )
    else()
        add_custom_command(OUTPUT "${_impl}" "${_header}"
          COMMAND ${HYPERSPACE_TOOLS_DIR}/hyperspace2cpp -a ${_basename} -i ${_include} -l ${_parentClass} ${_infile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          DEPENDS ${_infile}
          COMMENT "Generating Hyperspace Consumer ${_basename}.h, ${_basename}.cpp..." VERBATIM
        )
    endif()

    qt5_generate_moc("${_header}" "${_moc}")
    set_source_files_properties("${_impl}" PROPERTIES SKIP_AUTOMOC TRUE)
    macro_add_file_dependencies("${_impl}" "${_moc}")

    list(APPEND ${_sources} "${_impl}" "${_header}")
    set(${_sources} ${${_sources}} PARENT_SCOPE)

    install(FILES ${_infile} DESTINATION ${HYPERSPACE_INTERFACES_DIR})
endfunction(hyperspace_add_qt5_consumer)

function(hyperspace_add_qt5_producer _sources _interface_file) # _optionalBasename # _optionalClassName
    get_filename_component(_infile ${_interface_file} ABSOLUTE)
    if (NOT EXISTS ${_infile})
        message(FATAL_ERROR "Cannot find file ${_infile}. Aborting.")
    endif ()

    set(_optionalBasename "${ARGV2}")
    if(_optionalBasename)
        set(_basename ${_optionalBasename} )
    else()
        string(REGEX REPLACE "(.*[/\\.])?([^\\.]+)\\.json" "\\2producer" _basename ${_infile})
        string(TOLOWER ${_basename} _basename)
    endif()

    set(_optionalClassName "${ARGV3}")
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h")
    set(_impl   "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp")
    set(_moc    "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc")

    if(_optionalClassName)
        add_custom_command(OUTPUT "${_impl}" "${_header}"
          COMMAND ${HYPERSPACE_TOOLS_DIR}/hyperspace2cpp -a ${_basename} -c ${_optionalClassName} ${_infile}
          DEPENDS ${_infile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating Hyperspace Provider ${_basename}.h, ${_basename}.cpp..." VERBATIM
        )
    else()
        add_custom_command(OUTPUT "${_impl}" "${_header}"
          COMMAND ${HYPERSPACE_TOOLS_DIR}/hyperspace2cpp -a ${_basename} ${_infile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          DEPENDS ${_infile} VERBATIM
          COMMENT "Generating Hyperspace Provider ${_basename}.h, ${_basename}.cpp..." VERBATIM
        )
    endif()

    qt5_generate_moc("${_header}" "${_moc}")
    set_source_files_properties("${_impl}" PROPERTIES SKIP_AUTOMOC TRUE)
    macro_add_file_dependencies("${_impl}" "${_moc}")

    list(APPEND ${_sources} "${_impl}" "${_header}")
    set(${_sources} ${${_sources}} PARENT_SCOPE)

    install(FILES ${_infile} DESTINATION ${HYPERSPACE_INTERFACES_DIR})
endfunction(hyperspace_add_qt5_producer)
