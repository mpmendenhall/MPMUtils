cmake_minimum_required(VERSION 3.2)
project(MPMUtils)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_library(CONFIG_LIBS config++)
#find_package(HDF5 REQUIRED COMPONENTS C HL)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})
find_library(FFTW_LIBS fftw3)


# force rebuild of CodeVersion
#add_custom_command(OUTPUT always_rebuild COMMAND cmake -E echo)
add_custom_command(OUTPUT codeversion COMMAND touch ${PROJECT_SOURCE_DIR}/Utility/CodeVersion.cc)
add_custom_target(update_codeversion DEPENDS codeversion)

foreach(dir IN ITEMS
    Framework Math Utility Visualization
)
include_directories(${PROJECT_SOURCE_DIR}/${dir})
file(GLOB DIRSRC ${dir}/*.c*)
LIST(APPEND SOURCE_FILES ${DIRSRC})
endforeach(dir)

LIST(REMOVE_ITEM SOURCE_FILES ${PROJECT_SOURCE_DIR}/Utility/SQLite_Helper.cc)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib) # for ROOT_GENERATE_DICTIONARY

add_compile_options("-O3" "-fPIC" "-Wpedantic" "-Wall" "-std=c++11" "-DWITH_OPENGL")

################
# ROOT, ROOT Dicts
################

find_package(ROOT REQUIRED COMPONENTS MathCore Core RIO Hist Tree Minuit)
include_directories(${ROOT_INCLUDE_DIRS})
find_package(GSL)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

#set(DICTFILES "Event.hh" "CumulativeData.hh" "TCumulative.hh" "TCumulativeMap.hh" "TDynamicHistogram.hh")
#ROOT_GENERATE_DICTIONARY(P2x_RootDict ${DICTFILES} LINKDEF "FileIO/EventLinkDef.h")
#LIST(APPEND SOURCE_FILES P2x_RootDict.cxx)

################
# git SHA tag
################

exec_program( "git" ${PROJECT_SOURCE_DIR}
    ARGS "rev-parse -q HEAD"
    OUTPUT_VARIABLE GIT_SHA )
exec_program( "git" ${PROJECT_SOURCE_DIR}
    ARGS "describe --tags HEAD"
    OUTPUT_VARIABLE GIT_TAGNAME )
message("git repository '${GIT_TAGNAME}' (${GIT_SHA})")
SET_SOURCE_FILES_PROPERTIES(Utility/CodeVersion.cc PROPERTIES COMPILE_FLAGS "-DREPO_VERSION=\"${GIT_SHA}\" -DREPO_TAGNAME=\"${GIT_TAGNAME}\"")

################
# build targets
################

# libMPM omnibus
add_library(MPM SHARED ${SOURCE_FILES})
target_link_libraries(MPM ${CONFIG_LIBS} ${FFTW_LIBS} ${GLUT_LIBRARIES} ${GSL_LIBRARIES} ${OPENGL_LIBRARIES} ${ROOT_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})
add_dependencies(MPM update_codeversion)

# test programs
foreach(exec IN ITEMS testVisr testPolynomial)
    add_executable(${exec} Test/${exec}.cc)
    target_link_libraries(${exec} MPM)
endforeach(exec)

#################
# Documentation with Doxygen
#################

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
