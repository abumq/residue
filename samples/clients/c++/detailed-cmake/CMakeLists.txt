cmake_minimum_required(VERSION 2.8.12)

project(App)

set (APP_VERSION "1.0")
set (APP_SOVERSION "1.0")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include_directories (${CMAKE_BINARY_DIR})
include_directories (${CMAKE_SOURCE_DIR})

add_definitions (-DNON_ROOT_SAMPLE_BUILD)

option (BUILD_SHARED_LIBS "build shared libraries" OFF)
include(FindPackageHandleStandardArgs)

# We need C++11
macro(require_cpp11)
        if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
                # CMake 3.1 has built-in CXX standard checks.
                message("-- Setting C++11")
                set(CMAKE_CXX_STANDARD 11)
                set(CMAKE_CXX_STANDARD_REQUIRED on)
        else()
                if (CMAKE_CXX_COMPILER_ID MATCHES "GCC")
                    message ("-- GNU CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
                    message ("-- GNU CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                    message ("-- CLang CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                else()
                    message ("-- Requires C++11. Your compiler does not support it.")
                endif()
        endif()
endmacro()

require_cpp11()

# http://www.cmake.org/Wiki/CMake_RPATH_handling#Mac_OS_X_and_the_RPATH
if (APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    if("${isSystemDir}" STREQUAL "-1")
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    endif()
endif()

find_package(Residue REQUIRED)
include_directories (${RESIDUE_INCLUDE_DIR})

#######################################################

add_executable (sample-app-bin src/app.cc)

target_link_libraries (sample-app-bin
    ${RESIDUE_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties (sample-app-bin PROPERTIES
    OUTPUT_NAME "residue-sample-app"
    VERSION ${APP_VERSION}
    SOVERSION ${APP_SOVERSION}
)
