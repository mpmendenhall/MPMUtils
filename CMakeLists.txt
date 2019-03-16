cmake_minimum_required(VERSION 3.2)
project(MPMUtils)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib) # for ROOT_GENERATE_DICTIONARY

## how to require compile standards:
set(CMAKE_C_STANDARD 99) # sqlite3.c complains under c90 about 'long long' type
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("-fPIC" "-Wpedantic" "-Wall" "-ansi" "-pedantic-errors")
# cmake -DCMAKE_BUILD_TYPE=Release
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")
string(APPEND CMAKE_CXX_FLAGS " -ansi -pedantic-errors")

#####################
# locate dependencies

###########
# libconfig
###########
find_library(CONFIG_LIBS config++)
LIST(APPEND LINKLIBS ${CONFIG_LIBS})

########
# OpenGL
########
find_package(OpenGL)
find_package(GLUT)
if(GLUT_FOUND AND OPENGL_FOUND AND NOT NO_GL)
    add_compile_options("-DWITH_OPENGL")
    LIST(APPEND LINKLIBS ${OPENGL_LIBRARIES})
    LIST(APPEND LINKLIBS ${GLUT_LIBRARIES})
else()
    message("OpenGL dependencies disabled.")
endif()

#######
# BOOST
#######
find_package(Boost REQUIRED COMPONENTS fiber)
include_directories(${Boost_INCLUDE_DIR})
LIST(APPEND LINKLIBS ${Boost_LIBRARIES})

#####
# GSL
#####
find_package(GSL REQUIRED)
LIST(APPEND LINKLIBS ${GSL_LIBRARIES})

#######
# FFTW3
#######
find_library(FFTW_LIBS fftw3)
if(FFTW_LIBS-NOTFOUND OR NO_FFTW)
    message("Disabling FFTW3 dependency")
    add_compile_options("-DWITHOUT_FFTW")
else()
    LIST(APPEND LINKLIBS ${FFTW_LIBS})
endif()

#####
# MPI
#####
find_package(MPI)
if(MPI_FOUND AND NOT NO_MPI)
    add_compile_options("-DWITH_MPI")
    include_directories(${MPI_INCLUDE_PATH})
    LIST(APPEND LINKLIBS ${MPI_LIBRARIES})
    add_compile_options(${MPI_COMPILE_FLAGS})
else()
    message("MPI dependencies disabled.")
endif()

######
# ROOT
######
find_package(ROOT REQUIRED COMPONENTS MathCore MathMore Core RIO Hist Tree Minuit)
LIST(APPEND LINKLIBS ${ROOT_LIBRARIES})
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/ROOTUtils/)
include("${ROOT_USE_FILE}")
ROOT_GENERATE_DICTIONARY(mpmu_Dict "CumulativeData.hh" "TCumulative.hh" "TCumulativeMap.hh" "TDynamicHistogram.hh" LINKDEF "ROOTUtils/LinkDef.h" OPTIONS "")
list(APPEND SOURCE_FILES "mpmu_Dict.cxx")

######
# HDF5
######
find_package(HDF5 REQUIRED COMPONENTS C HL)
message("Including HDF5 paths '${HDF5_INCLUDE_DIRS}' and libraries '${HDF5_LIBRARIES}' in '${HDF5_LIBRARY_DIRS}'")
include_directories(${HDF5_INCLUDE_DIRS})
LIST(APPEND LINKLIBS ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})


########
# Geant4
########
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()
include(${Geant4_USE_FILE})
Message("Geant4_USE_FILE = ${Geant4_USE_FILE}")
LIST(APPEND LINKLIBS ${Geant4_LIBRARIES})


##############################
# force rebuild of CodeVersion
#add_custom_command(OUTPUT always_rebuild COMMAND cmake -E echo)
add_custom_command(OUTPUT codeversion COMMAND touch ${PROJECT_SOURCE_DIR}/Utility/CodeVersion.cc)
add_custom_target(update_codeversion DEPENDS codeversion)

#########################
# Choose objects to build

foreach(dir IN ITEMS Framework GeneralUtils JobControl Math Matrix Physics ROOTUtils Utility Visualization )
include_directories(${PROJECT_SOURCE_DIR}/${dir})
file(GLOB DIRSRC ${dir}/*.c*)
LIST(APPEND SOURCE_FILES ${DIRSRC})
endforeach(dir)

# skip broken misc crud
foreach(RM IN ITEMS GeneralUtils/Sketch3D.cc Matrix/TLS_Solver.cc)
LIST(REMOVE_ITEM SOURCE_FILES ${PROJECT_SOURCE_DIR}/${RM})
endforeach()

# additional required libraries
LIST(APPEND LINKLIBS -lpthread)


################
# git SHA tag
################

exec_program("git" ${PROJECT_SOURCE_DIR}
    ARGS "rev-parse -q HEAD"
    OUTPUT_VARIABLE GIT_SHA )
exec_program("git" ${PROJECT_SOURCE_DIR}
    ARGS "describe --tags HEAD"
    OUTPUT_VARIABLE GIT_TAGNAME )
exec_program("basename" ${PROJECT_SOURCE_DIR}
    ARGS "`git rev-parse --show-toplevel`"
    OUTPUT_VARIABLE REPO_NAME)
message("git repo ${REPO_NAME} '${GIT_TAGNAME}' (${GIT_SHA})")
SET_SOURCE_FILES_PROPERTIES(
    Utility/CodeVersion.cc PROPERTIES COMPILE_FLAGS
    "-DREPO_NAME=\"${REPO_NAME}\" -DREPO_VERSION=\"${GIT_SHA}\" -DREPO_TAGNAME=\"${GIT_TAGNAME}\""
)

################
# build targets
################

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_options("-O")
endif()

# libMPM omnibus
add_library(MPM SHARED ${SOURCE_FILES})
target_link_libraries(MPM ${LINKLIBS})
add_dependencies(MPM update_codeversion)

# test programs
file(GLOB TESTEXEC Test/*.cc)
foreach(E ${TESTEXEC})
get_filename_component(EXN ${E} NAME_WE)
    add_executable(${EXN} ${E})
    target_link_libraries(${EXN} MPM)
endforeach(E)

#################
# Documentation with Doxygen
#################

#################
# Documentation with Doxygen
#################
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
