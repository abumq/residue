cmake_minimum_required (VERSION 2.8.12)

project (Residue)

option (test "Build all tests" OFF)
option (travis "Travis CI" OFF)
option (production "Build for production" ON)
option (debug "Enable debug logging" OFF)
option (build_static_lib "Build residue as a static library (as opposed to default, i.e, shared)" OFF)
option (build_sample_app "Builds detailed-cmake sample" OFF)
option (profiling "Turns profiling on for various scenarios" OFF)
option (compile-server "Turn it on when you have source code available for the server" OFF)
option (static_ripe "Prefer static ripe library" OFF)
option (BUILD_SHARED_LIBS "build shared libraries" OFF)

set (RESIDUE_MAJOR "1")
set (RESIDUE_MINOR "0")
set (RESIDUE_PATCH "0-beta.14")
set (RESIDUE_VERSION "${RESIDUE_MAJOR}.${RESIDUE_MINOR}.${RESIDUE_PATCH}")
set (RESIDUE_SOVERSION "${RESIDUE_MAJOR}.${RESIDUE_MINOR}.${RESIDUE_PATCH}")
set (RESIDUE_NAME "Residue")

add_definitions (-DRESIDUE_VERSION="${RESIDUE_VERSION}")
add_definitions (-DRESIDUE_SOVERSION="${RESIDUE_SOVERSION}")

# set(CMAKE_MACOSX_RPATH 1)

# We need C++11
macro (require_cpp11)
        if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
                # CMake 3.1 has built-in CXX standard checks.
                message ("-- Setting C++11")
                set (CMAKE_CXX_STANDARD 11)
                set (CMAKE_CXX_STANDARD_REQUIRED on)
        else()
                if (CMAKE_CXX_COMPILER_ID MATCHES "GCC")
                    message ("-- GNU CXX (-std=c++11)")
                    list (APPEND CMAKE_CXX_FLAGS "-std=c++11")
                elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
                    message ("-- GNU CXX (-std=c++11)")
                    list (APPEND CMAKE_CXX_FLAGS "-std=c++11")
                elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                    message ("-- CLang CXX (-std=c++11)")
                    list (APPEND CMAKE_CXX_FLAGS "-std=c++11")
                else()
                    message ("-- Requires C++11. Your compiler does not support it.")
                endif()
        endif()
endmacro()

if (profiling)
    add_definitions (-DRESIDUE_PROFILING)
endif()

if (debug)
    message ("-- DEBUGGING IS ON")
    add_definitions (-DRESIDUE_DEBUG)
else()
    message ("-- DEBUGGING IS OFF")
endif()

if (production)
    message ("-- PRODUCTION BUILD")
    add_definitions (-DRESIDUE_PRODUCTION)
endif()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (FindPackageHandleStandardArgs)

require_cpp11()

# http://www.cmake.org/Wiki/CMake_RPATH_handling#Mac_OS_X_and_the_RPATH
if (APPLE)
    set (CMAKE_MACOSX_RPATH ON)
    set (CMAKE_SKIP_BUILD_RPATH FALSE)
    set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    list (FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    if ("${isSystemDir}" STREQUAL "-1")
        set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    endif()
endif()

if (travis)
else()
    list (APPEND CMAKE_CXX_FLAGS "-Wno-deprecated-declarations")
endif()

message ("-- Check for Easylogging++")
find_package (EASYLOGGINGPP REQUIRED)
include_directories (${EASYLOGGINGPP_INCLUDE_DIR})

set (Ripe_USE_STATIC_LIBS ${static_ripe})
find_package (Ripe REQUIRED)
include_directories (${RIPE_INCLUDE_DIR})

message ("-- Check for Boost System (static)")
## We always use static linking for boost stuffs
set (Boost_USE_STATIC_LIBS OFF) ## TODO: Fix this and turn it off
find_package (Boost REQUIRED COMPONENTS system)
include_directories (${Boost_INCLUDE_DIR})
message ("-- Boost binary: " ${Boost_LIBRARIES})

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


############# RESIDUE CLIENT LIB ###############

set(LIB_RESIDUE_SOURCE_FILES
    lib/Residue.cc
    ${EASYLOGGINGPP_INCLUDE_DIR}/easylogging++.cc
)

if (build_static_lib)
    add_library (residue-lib STATIC ${LIB_RESIDUE_SOURCE_FILES})
else()
    add_library (residue-lib SHARED ${LIB_RESIDUE_SOURCE_FILES})
endif()

set_target_properties (residue-lib PROPERTIES
    VERSION ${RESIDUE_VERSION}
    SOVERSION ${RESIDUE_SOVERSION}
)
target_link_libraries (residue-lib
    ${Boost_LIBRARIES}
    ${RIPE_LIBRARY}
)

target_compile_definitions (residue-lib PRIVATE
    ELPP_FORCE_USE_STD_THREAD
    ELPP_THREAD_SAFE
    ELPP_NO_LOG_TO_FILE
    ELPP_NO_DEFAULT_LOG_FILE
    ELPP_DEFAULT_LOGGING_FLAGS=4096
    ELPP_FEATURE_ALL
    ELPP_STL_LOGGING
    ELPP_LOG_STD_ARRAY
    ELPP_LOG_UNORDERED_MAP
    ELPP_LOG_UNORDERED_SET
)

set_target_properties (residue-lib PROPERTIES OUTPUT_NAME "residue")
install (TARGETS residue-lib DESTINATION lib)
install (FILES include/Residue.h DESTINATION "include")


############# SAMPLE CLIENT APP ###############

if (build_sample_app)
    add_subdirectory (samples/clients/c++/detailed-cmake/)
endif()

# Packaging config.
set (CPACK_PACKAGE_NAME "Residue")
set (CPACK_PACKAGE_VERSION_MAJOR ${RESIDUE_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${RESIDUE_MINOR})
set (CPACK_SOURCE_GENERATOR TGZ)
set (CPACK_SOURCE_IGNORE_FILES
    "/build/"
)
include (CPack)

# Compile-time configuration.
configure_file (
    ${CMAKE_SOURCE_DIR}/cmake/config.h.cmake
    ${CMAKE_BINARY_DIR}/config.h
)

include_directories (${CMAKE_BINARY_DIR})
include_directories (${CMAKE_SOURCE_DIR})


############# SERVER ###############

if (compile-server)
    set (SOURCE_FILES
        src/Url.cc
        src/Utils.cc
        src/Configuration.cc
        src/Registry.cc
        src/RequestHandler.cc
        src/TokenRequestHandler.cc
        src/LogRequestHandler.cc
        src/ConnectionRequestHandler.cc
        src/AdminRequestHandler.cc
        src/Session.cc
        src/JsonObject.cc
        src/Request.cc
        src/Response.cc
        src/AdminRequest.cc
        src/ConnectionRequest.cc
        src/ConnectionResponse.cc
        src/LogRequest.cc
        src/TokenRequest.cc
        src/Server.cc
        src/UserLogBuilder.cc
        src/Token.cc
        src/Client.cc
        src/TokenResponse.cc
        src/ClientIntegrityTask.cc
        src/LogRotator.cc
        src/Task.cc
        src/CommandHandler.cc
        src/Tar.cc
    )

    set (SHARED_REQUIRED_LIBS
        ${RIPE_LIBRARY}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )

    ############# RESIDUE CORE LIB (STATIC) ###############

    add_library (residue-core ${SOURCE_FILES})

    set_target_properties (residue-core PROPERTIES
        VERSION ${RESIDUE_VERSION}
        SOVERSION ${RESIDUE_SOVERSION}
    )

    target_link_libraries (residue-core ${SHARED_REQUIRED_LIBS})

    target_compile_definitions (residue-core PUBLIC
        ELPP_FORCE_USE_STD_THREAD
        ELPP_THREAD_SAFE
    )

    # Set RPATH to library install path.
    set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")


    ############# CONFIG VALIDATOR TOOL ###############

    #add_executable (config-validator-bin
    #    tools/config-validator/config-validator.cc
    #    ${EASYLOGGINGPP_INCLUDE_DIR}/easylogging++.cc
    #)

    #target_compile_definitions (config-validator-bin PUBLIC
    #    ELPP_NO_DEFAULT_LOG_FILE
    #    ELPP_NO_LOG_TO_FILE
    #    ELPP_STL_LOGGING
    #    AUTO_INITIALIZE_EASYLOGGINGPP
    #    ELPP_DEFAULT_LOGGING_FLAGS=4096
    #)

    #target_link_libraries (config-validator-bin ${RIPE_LIBRARY} residue-core)

    #set_target_properties (config-validator-bin PROPERTIES
    #    OUTPUT_NAME "config-validator"
    #    VERSION ${RESIDUE_VERSION}
    #    SOVERSION ${RESIDUE_SOVERSION}
    #)

    # -- DO NOT UNCOMMENT install (TARGETS config-validator-bin DESTINATION bin)


    ############# RESIDUE BINARY ###############

    add_executable (residue-bin
        src/main.cc
        ${EASYLOGGINGPP_INCLUDE_DIR}/easylogging++.cc
    )

    target_compile_definitions (residue-bin PUBLIC
        ELPP_STL_LOGGING
        ELPP_STACKTRACE
        ELPP_FEATURE_CRASH_LOG
        ELPP_DEFAULT_LOG_FILE="/tmp/logs/residue_default_logger.log"
    )

    set_target_properties (residue-bin PROPERTIES
        VERSION ${RESIDUE_VERSION}
        SOVERSION ${RESIDUE_SOVERSION}
        OUTPUT_NAME "residue"
    )

    target_link_libraries (residue-bin residue-core)

    set_target_properties (residue-bin PROPERTIES OUTPUT_NAME "residue")
    install (TARGETS residue-bin DESTINATION bin)

    #############################################

    # Reference all headers, to make certain IDEs happy.
    file (GLOB_RECURSE all_headers ${CMAKE_SOURCE_DIR}/*.h)
    add_custom_target (all_placeholder SOURCES ${all_headers})


    ############# RESIDUE TESTS ###############
    if (test)

        find_package (gtest REQUIRED)

        include_directories (${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

        enable_testing()

        add_executable (residue-unit-tests
            test/main.cc
            ${EASYLOGGINGPP_INCLUDE_DIR}/easylogging++.cc
        )

        target_compile_definitions (residue-unit-tests PUBLIC
            ELPP_FEATURE_ALL
            ELPP_DEFAULT_LOG_FILE="/tmp/logs/residue_unit_tests.log"
            ELPP_DEFAULT_LOGGING_FLAGS=4096
        )

        # Standard linking to gtest stuff.
        target_link_libraries (residue-unit-tests gtest gtest_main)

        # Extra linking for the project.
        target_link_libraries (residue-unit-tests residue-core)

        target_link_libraries (residue-unit-tests ${SHARED_REQUIRED_LIBS})

        add_test (NAME residueUnitTests COMMAND residue-unit-tests)
    endif() ## test
endif() ## Compile server
