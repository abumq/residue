
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

macro (check_apple)
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
endmacro()

macro (thread_packages_check)

    # Check for include files and stdlib properties.
    include (CheckIncludeFileCXX)
    check_include_file_cxx (attr/xattr.h HAVE_ATTR_XATTR_H)
    check_include_file_cxx (sys/xattr.h HAVE_SYS_XATTR_H)

    # Check if xattr functions take extra arguments, as they do on OSX.
    # Output error is misleading, so do this test quietly.
    include (CheckCXXSourceCompiles)
    set (CMAKE_REQUIRED_QUIET_SAVE ${CMAKE_REQUIRED_QUIET})
    set (CMAKE_REQUIRED_QUIET True)
    check_cxx_source_compiles ("#include <sys/types.h>
    #include <sys/xattr.h>
    int main() { getxattr(0,0,0,0,0,0); return 1; }
    " XATTR_ADD_OPT)
    set (CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_SAVE})

    set (CMAKE_THREAD_PREFER_PTHREAD)
    find_package (Threads REQUIRED)
endmacro()
