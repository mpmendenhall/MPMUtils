
add_custom_target(doc)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    configure_file(Doxyfile Doxyfile @ONLY)
    add_custom_target(doxydoc
        ${DOXYGEN_EXECUTABLE} Doxyfile
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    add_dependencies(doc doxydoc)
endif(DOXYGEN_FOUND)

