
add_custom_target(doc)

# project Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    configure_file(Doxyfile Doxyfile @ONLY)
    add_custom_target(doxydoc
        ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Doc/
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    add_dependencies(doc doxydoc)
endif(DOXYGEN_FOUND)

# run "cppcheck" static code analysis on project
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(cppcheck
        cppcheck --project=compile_commands.json --enable=all --quiet --suppress=missingIncludeSystem
            -i ${CMAKE_BINARY_DIR} -i ${CMAKE_SOURCE_DIR}/External/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()
