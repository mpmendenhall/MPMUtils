cmake_minimum_required(VERSION 3.2)
project(MPMUtils)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib) # for ROOT_GENERATE_DICTIONARY

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
add_compile_options("-fPIC" "-Wpedantic" "-Wall" $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3")

#####################
# locate dependencies

############
find_library(CONFIG_LIBS config++)
LIST(APPEND LINKLIBS ${CONFIG_LIBS})

############
find_package(OpenGL)
find_package(GLUT)
if(GLUT_FOUND AND OPENGL_FOUND AND NOT NO_GL)
    add_compile_options("-DWITH_OPENGL")
    LIST(APPEND LINKLIBS ${OPENGL_LIBRARIES})
    LIST(APPEND LINKLIBS ${GLUT_LIBRARIES})
else()
    message("OpenGL dependencies disabled.")
endif()

############
find_package(Boost REQUIRED COMPONENTS fiber)
include_directories(${Boost_INCLUDE_DIR})
LIST(APPEND LINKLIBS ${Boost_LIBRARIES})

############
find_package(GSL REQUIRED)
LIST(APPEND LINKLIBS ${GSL_LIBRARIES})

############
find_library(FFTW_LIBS fftw3)
if(FFTW_LIBS-NOTFOUND OR NO_FFTW)
    message("Disabling FFTW3 dependency")
    add_compile_options("-DWITHOUT_FFTW")
else()
    LIST(APPEND LINKLIBS ${FFTW_LIBS})
endif()

############
find_package(MPI)
if(MPI_FOUND AND NOT NO_MPI)
    add_compile_options("-DWITH_MPI")
    include_directories(${MPI_INCLUDE_PATH})
    LIST(APPEND LINKLIBS ${MPI_LIBRARIES})
    add_compile_options(${MPI_COMPILE_FLAGS})
else()
    message("MPI dependencies disabled.")
endif()

############
find_package(ROOT REQUIRED COMPONENTS MathCore MathMore Core RIO Hist Tree Minuit)
LIST(APPEND LINKLIBS ${ROOT_LIBRARIES})
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/ROOTUtils/)
include("${ROOT_USE_FILE}")
ROOT_GENERATE_DICTIONARY(mpmu_Dict "CumulativeData.hh" "TCumulative.hh" "TCumulativeMap.hh" "TDynamicHistogram.hh" LINKDEF "ROOTUtils/LinkDef.h" OPTIONS "")

######
# HDF5
######
find_package(HDF5 REQUIRED COMPONENTS C HL)
message("Including HDF5 paths '${HDF5_INCLUDE_DIRS}' and libraries '${HDF5_LIBRARIES}' in '${HDF5_LIBRARY_DIRS}'")
include_directories(${HDF5_INCLUDE_DIRS})
LIST(APPEND LINKLIBS ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})


##############################
# force rebuild of CodeVersion
#add_custom_command(OUTPUT always_rebuild COMMAND cmake -E echo)
add_custom_command(OUTPUT codeversion COMMAND touch ${PROJECT_SOURCE_DIR}/Utility/CodeVersion.cc)
add_custom_target(update_codeversion DEPENDS codeversion)

#########################
# Choose objects to build

foreach(dir IN ITEMS Framework GeneralUtils JobControl Math Matrix ROOTUtils Utility Visualization )
include_directories(${PROJECT_SOURCE_DIR}/${dir})
file(GLOB DIRSRC ${dir}/*.c*)
LIST(APPEND SOURCE_FILES ${DIRSRC})
endforeach(dir)

list(APPEND SOURCE_FILES "mpmu_Dict.cxx")

# skip broken misc crud
foreach(RM IN ITEMS GeneralUtils/Sketch3D.cc Matrix/TLS_Solver.cc)
LIST(REMOVE_ITEM SOURCE_FILES ${PROJECT_SOURCE_DIR}/${RM})
endforeach()

LIST(APPEND SOURCE_FILES Physics/BetaSpectrumGenerator.cc Physics/PolarizedBetaAsym.cc Physics/RelKin.cc Physics/UnpolarizedBeta.cc)


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

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
