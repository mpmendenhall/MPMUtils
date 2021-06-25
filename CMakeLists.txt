cmake_minimum_required(VERSION 3.2)
project(MPMUtils)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib) # for ROOT_GENERATE_DICTIONARY

## how to require compile standards:
set(CMAKE_C_STANDARD 99) # sqlite3.c complains under c90 about 'long long' type
set(CMAKE_C_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_STANDARD 17) # 'constexpr' getting usable
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE(CXXWarnings.cmake)
INCLUDE(DefaultBuildType.cmake)

string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")

#####################
# locate dependencies

###########
# libconfig
###########
find_library(CONFIG_LIBS config++)
LIST(APPEND EXTLIBS ${CONFIG_LIBS})

#########
# libuuid
#########
find_library(UUID_LIB uuid)
find_path(UUID_INCLUDE uuid/uuid.h)
message(STATUS "uuid/uuid.h headers in " ${UUID_INCLUDE} ", library " ${UUID_LIB})
if(UUID_INCLUDE AND UUID_LIB)
    include_directories(${UUID_INCLUDE})
    list(APPEND EXTLIBS ${UUID_LIB})
else()
    message(FATAL_ERROR "libuuid library or headers not found --- maybe install libuuid-devel package?")
endif()

########
# OpenGL
########
set(OpenGL_GL_PREFERENCE "GLVND" CACHE STRING "OpenGL version option, LEGACY or GLVND" FORCE)
find_package(GLUT)
find_package(OpenGL)
if(${GLUT_FOUND} AND ${OPENGL_FOUND})
    include_directories(SYSTEM ${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
    list(APPEND EXTLIBS ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
    list(APPEND CXXOPTS "-DWITH_OPENGL")
endif()

#######
# BOOST
#######
find_package(Boost REQUIRED COMPONENTS fiber)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
LIST(APPEND EXTLIBS ${Boost_LIBRARIES})

#####
# GSL
#####
find_package(GSL REQUIRED)
LIST(APPEND EXTLIBS ${GSL_LIBRARIES})

#######
# FFTW3
#######
find_library(FFTW_LIBS fftw3)
if(FFTW_LIBS-NOTFOUND OR NO_FFTW)
    message(STATUS "Disabling FFTW3 dependency")
    add_compile_options("-DWITHOUT_FFTW")
else()
    add_compile_options("-DWITH_FFTW_FLOAT128")
    LIST(APPEND EXTLIBS ${FFTW_LIBS} fftw3f fftw3l fftw3q quadmath m)
endif()

#####
# MPI
#####
find_package(MPI)
if(MPI_FOUND AND NOT NO_MPI)
    add_compile_options("-DWITH_MPI")
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
    LIST(APPEND EXTLIBS ${MPI_LIBRARIES})
    add_compile_options(${MPI_COMPILE_FLAGS})
else()
    message(STATUS "MPI dependencies disabled.")
endif()

######
# ROOT
######
find_package(ROOT REQUIRED COMPONENTS MathCore MathMore Core RIO Hist Tree Minuit)
LIST(APPEND EXTLIBS ${ROOT_LIBRARIES})
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/ROOTUtils/)
include("${ROOT_USE_FILE}")
ROOT_GENERATE_DICTIONARY(mpmu_Dict "CumulativeData.hh" "TCumulative.hh" "TCumulativeMap.hh" "TDynamicHistogram.hh" LINKDEF "ROOTUtils/LinkDef.h" OPTIONS "")
list(APPEND SOURCE_FILES "mpmu_Dict.cxx")

######
# HDF5
######
find_package(HDF5 REQUIRED COMPONENTS C HL)
message(STATUS "Including HDF5 paths '${HDF5_INCLUDE_DIRS}' and libraries '${HDF5_LIBRARIES}' in '${HDF5_LIBRARY_DIRS}'")
include_directories(SYSTEM ${HDF5_INCLUDE_DIRS})
LIST(APPEND EXTLIBS ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})


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
Message(STATUS "Geant4_USE_FILE = ${Geant4_USE_FILE}")
LIST(APPEND EXTLIBS ${Geant4_LIBRARIES})

#########################
# Choose objects to build

foreach(dir IN ITEMS External Exegete Framework JobControl Math Matrix Physics ROOTUtils Utility Visualization )
include_directories(${PROJECT_SOURCE_DIR}/${dir})
file(GLOB DIRSRC ${dir}/*.c*)
LIST(APPEND SOURCE_FILES ${DIRSRC})
endforeach(dir)

# additional required libraries
find_library(LIB_PTHREAD pthread REQUIRED)
list(APPEND EXTLIBS ${LIB_PTHREAD})

find_library(LAPACKE_LIBS lapacke)
if(LAPACKE_LIBS)
    MESSAGE(STATUS "lapacke library ${LAPACKE_LIBS}")
    add_compile_options("-DWITH_LAPACKE")
    LIST(APPEND EXTLIBS ${LAPACKE_LIBS})
else()
    LIST(REMOVE_ITEM SOURCE_FILES ${PROJECT_SOURCE_DIR}/Matrix/TLS_Solver.cc)
endif()


################
# git SHA tag
################

INCLUDE(GitRepoTag.cmake)
SET_SOURCE_FILES_PROPERTIES(Framework/CodeVersion.cc PROPERTIES COMPILE_FLAGS ${CODEVERSION_COMPILE_FLAGS})
# force rebuild of CodeVersion
add_custom_command(OUTPUT codeversion COMMAND touch ${PROJECT_SOURCE_DIR}/Framework/CodeVersion.cc)
add_custom_target(update_codeversion DEPENDS codeversion)

################
# build targets
################

# CXXFLAGS compile options, for c++ only.
LIST(REMOVE_DUPLICATES CXXOPTS)
message(STATUS "c++ compiler flags:")
FOREACH(CXXOPT ${CXXOPTS})
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${CXXOPT}>)
    message(STATUS "\t${CXXOPT}")
ENDFOREACH()

LIST(REMOVE_DUPLICATES EXTLIBS)
message(STATUS "Link libraries:")
foreach(l ${EXTLIBS})
    message(STATUS "\t${l}")
endforeach(l)

# libMPM omnibus
add_library(MPM SHARED ${SOURCE_FILES})
target_link_libraries(MPM ${EXTLIBS})
add_dependencies(MPM update_codeversion)

# test programs
file(GLOB TESTEXEC Test/*.cc)
foreach(E ${TESTEXEC})
get_filename_component(EXN ${E} NAME_WE)
    add_executable(${EXN} ${E})
    target_link_libraries(${EXN} MPM ${EXTLIBS})
endforeach(E)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(Doc)
