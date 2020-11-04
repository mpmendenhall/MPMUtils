LIST(APPEND CXXOPTS "-Wall" "-Wextra")
option(WERROR "Enable/disable -Werror compile flags" ON)
IF(WERROR)
    MESSAGE(STATUS "Enabling -Werror compile flag, turning warnings into errors")
    LIST(APPEND CXXOPTS "-Werror")
endif()

IF(CMAKE_COMPILER_IS_GNUCC)
    LIST(APPEND CXXOPTS "-Wpedantic" "-Wlogical-op" "-Wshadow" "-Wformat=2" "-pedantic-errors")

    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0")
        # regression bug in gcc6.0,... throws warnings on inherited constructors
        LIST(APPEND CXXOPTS "-Wuseless-cast")
    else()
        LIST(APPEND CXXOPTS "-Wduplicated-cond" "-Wmissing-include-dirs" "-Wredundant-decls"
        "-Wnull-dereference" "-Wsuggest-override" "-Wuninitialized" "-Winit-self")

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
            LIST(APPEND CXXOPTS "-Wduplicated-branches" "-Woverloaded-virtual")
        endif()

        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.3")
            LIST(APPEND CXXOPTS "-Wuseless-cast")
        endif()
    endif()

endif()
