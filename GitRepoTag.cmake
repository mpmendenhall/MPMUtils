set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/.git)

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

    SET(CODEVERSION_COMPILE_DEFS "REPO_NAME=${REPO_NAME};REPO_VERSION=${GIT_SHA};REPO_TAGNAME=${GIT_TAGNAME}")
    message(STATUS "git repo ${REPO_NAME} '${GIT_TAGNAME}' (${GIT_SHA})")

else()

    message(WARNING "Project source directory not tracked by version control")
    SET(CODEVERSION_COMPILE_DEFS "REPO_NAME=untracked;REPO_VERSION=unversioned;REPO_TAGNAME=untagged")

endif()
