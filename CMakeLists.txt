cmake_minimum_required(VERSION 3.2)
project(MPMUtils)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_library(CONFIG_LIBS config++)
find_package(HDF5 REQUIRED COMPONENTS C HL)
include_directories(${HDF5_INCLUDE_DIRS})
find_library(FFTW_LIBS fftw3)

foreach(dir IN ITEMS
    Framework Math Utility Visualization
)
include_directories(${PROJECT_SOURCE_DIR}/${dir})
file(GLOB DIRSRC ${dir}/*.c*)
LIST(APPEND SOURCE_FILES ${DIRSRC})
endforeach(dir)

LIST(REMOVE_ITEM SOURCE_FILES ${PROJECT_SOURCE_DIR}/Utility/SQLite_Helper.cc)
message(${SOURCE_FILES})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib) # for ROOT_GENERATE_DICTIONARY

add_compile_options("-O3" "-fPIC" "-Wpedantic" "-Wall" "-std=c++11")

################
# ROOT, ROOT Dicts
################

find_package(ROOT REQUIRED COMPONENTS MathCore Core RIO Hist Tree Minuit)
include_directories(${ROOT_INCLUDE_DIRS})

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

add_library(MPM SHARED ${SOURCE_FILES})
target_link_libraries(MPM ${ROOT_LIBRARIES} ${CONFIG_LIBS} ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES} ${FFTW_LIBS})

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
