exec_program( "git" ${PROJECT_SOURCE_DIR}
    ARGS "rev-parse -q HEAD"
    OUTPUT_VARIABLE GIT_SHA
    RETURN_VALUE GIT_SHA_RET)

if(NOT ${GIT_SHA_RET})

    exec_program( "git" ${PROJECT_SOURCE_DIR}
        ARGS "describe --tags HEAD"
        OUTPUT_VARIABLE GIT_TAGNAME )
    exec_program("basename" ${PROJECT_SOURCE_DIR}
        ARGS "`git rev-parse --show-toplevel`"
        OUTPUT_VARIABLE REPO_NAME)
    message(STATUS "git repo ${REPO_NAME} '${GIT_TAGNAME}' (${GIT_SHA})")

    SET(CODEVERSION_COMPILE_FLAGS "-DREPO_NAME=\"${REPO_NAME}\" -DREPO_VERSION=\"${GIT_SHA}\" -DREPO_TAGNAME=\"${GIT_TAGNAME}\"")

else()

    message(WARNING "Project source directory not tracked by version control")
    SET(CODEVERSION_COMPILE_FLAGS "-DREPO_NAME=unknown -DREPO_VERSION=unknown -DREPO_TAGNAME=unknown")

endif()
