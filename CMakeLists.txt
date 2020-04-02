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
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# c++11 is "minimum" standard for codebase; ROOT may specify newer below.
LIST(APPEND CXXOPTS "-fPIC" "-std=c++11" "-Wall" "-Wextra" "-Werror")

IF(CMAKE_COMPILER_IS_GNUCC)
    LIST(APPEND CXXOPTS "-Wpedantic" "-Wlogical-op" "-Wshadow" "-Wformat=2" "-pedantic-errors" "-Wuseless-cast")

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0")
        LIST(APPEND CXXOPTS "-Wduplicated-cond" "-Wmissing-include-dirs" "-Wredundant-decls"
        "-Wnull-dereference" "-Wsuggest-override")
    endif()
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
        LIST(APPEND CXXOPTS "-Wduplicated-branches" "-Woverloaded-virtual")
    endif()
endif()


# cmake -DCMAKE_BUILD_TYPE=Release
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")

#####################
# locate dependencies

###########
# libconfig
###########
find_library(CONFIG_LIBS config++)
LIST(APPEND LINKLIBS ${CONFIG_LIBS})

#########
# libuuid
#########
find_library(UUID_LIBS uuid)
LIST(APPEND LINKLIBS ${UUID_LIBS})

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
    message(STATUS "OpenGL dependencies disabled.")
endif()

#######
# BOOST
#######
find_package(Boost REQUIRED COMPONENTS fiber)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
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
    message(STATUS "Disabling FFTW3 dependency")
    add_compile_options("-DWITHOUT_FFTW")
else()
    add_compile_options("-DWITH_FFTW_FLOAT128")
    LIST(APPEND LINKLIBS ${FFTW_LIBS} fftw3f fftw3l fftw3q quadmath m)
endif()

#####
# MPI
#####
find_package(MPI)
if(MPI_FOUND AND NOT NO_MPI)
    add_compile_options("-DWITH_MPI")
    include_directories(SYSTEM ${MPI_INCLUDE_PATH})
    LIST(APPEND LINKLIBS ${MPI_LIBRARIES})
    add_compile_options(${MPI_COMPILE_FLAGS})
else()
    message(STATUS "MPI dependencies disabled.")
endif()

######
# ROOT
######
find_package(ROOT REQUIRED COMPONENTS MathCore MathMore Core RIO Hist Tree Minuit)
LIST(APPEND LINKLIBS ${ROOT_LIBRARIES})
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
Message(STATUS "Geant4_USE_FILE = ${Geant4_USE_FILE}")
LIST(APPEND LINKLIBS ${Geant4_LIBRARIES})


##############################
# force rebuild of CodeVersion
#add_custom_command(OUTPUT always_rebuild COMMAND cmake -E echo)
add_custom_command(OUTPUT codeversion COMMAND touch ${PROJECT_SOURCE_DIR}/Framework/CodeVersion.cc)
add_custom_target(update_codeversion DEPENDS codeversion)

#########################
# Choose objects to build

foreach(dir IN ITEMS External Framework JobControl Math Matrix Physics ROOTUtils Utility Visualization )
include_directories(${PROJECT_SOURCE_DIR}/${dir})
file(GLOB DIRSRC ${dir}/*.c*)
LIST(APPEND SOURCE_FILES ${DIRSRC})
endforeach(dir)

# additional required libraries
LIST(APPEND LINKLIBS -lpthread)

find_library(LAPACKE_LIBS lapacke)
if(LAPACKE_LIBS)
    MESSAGE(STATUS "lapacke library ${LAPACKE_LIBS}")
    add_compile_options("-DWITH_LAPACKE")
    LIST(APPEND LINKLIBS ${LAPACKE_LIBS})
else()
    LIST(REMOVE_ITEM SOURCE_FILES ${PROJECT_SOURCE_DIR}/Matrix/TLS_Solver.cc)
endif()


################
# git SHA tag
################

execute_process(COMMAND "git" "rev-parse" "-q" "HEAD"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "git" "describe" "--tags" "HEAD"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAGNAME
    OUTPUT_STRIP_TRAILING_WHITESPACE)
exec_program("basename" ${PROJECT_SOURCE_DIR}
    ARGS "`git rev-parse --show-toplevel`"
    OUTPUT_VARIABLE REPO_NAME)
message(STATUS "git repo \"${REPO_NAME}\" '${GIT_TAGNAME}' (${GIT_SHA})")
SET_SOURCE_FILES_PROPERTIES(
    Framework/CodeVersion.cc PROPERTIES COMPILE_DEFINITIONS
    "REPO_NAME=\"${REPO_NAME}\";REPO_VERSION=\"${GIT_SHA}\";REPO_TAGNAME=\"${GIT_TAGNAME}\""
)

################
# build targets
################

# CXXFLAGS compile options, for c++ only.
FOREACH(CXXOPT ${CXXOPTS})
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${CXXOPT}>)
ENDFOREACH()

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
    target_link_libraries(${EXN} MPM ${LINKLIBS})
endforeach(E)

add_subdirectory(doc)

# run "cppcheck" static code analysis on project
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(cppcheck
    cppcheck --project=compile_commands.json --enable=all --quiet -i${PROJECT_SOURCE_DIR}/External -i${PROJECT_BINARY_DIR}/
)
