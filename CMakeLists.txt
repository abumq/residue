cmake_minimum_required (VERSION 2.8.12)

project (Residue)

option (test "Build all tests" OFF)
option (travis "Travis CI" OFF)
option (production "Build for production" ON)
option (debug "Enable debug logging" OFF)
option (profiling "Turns profiling on for various scenarios" OFF)
option (BUILD_SHARED_LIBS "build shared libraries" ON)

set (RESIDUE_MAJOR "1")
set (RESIDUE_MINOR "0")
set (RESIDUE_PATCH "0-beta.14")
set (RESIDUE_VERSION "${RESIDUE_MAJOR}.${RESIDUE_MINOR}.${RESIDUE_PATCH}")
set (RESIDUE_NAME "Residue")

add_definitions (-DRESIDUE_VERSION="${RESIDUE_VERSION}")

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
include(residue-dev)

include (FindPackageHandleStandardArgs)

require_cpp11()
check_apple()

if (travis)
else()
    list (APPEND CMAKE_CXX_FLAGS "-Wno-deprecated-declarations")
endif()

message ("-- Check for Easylogging++")
find_package (EASYLOGGINGPP REQUIRED)
include_directories (${EASYLOGGINGPP_INCLUDE_DIR})

set (Ripe_USE_STATIC_LIBS OFF)
find_package (Ripe REQUIRED)
include_directories (${RIPE_INCLUDE_DIR})

message ("-- Check for Boost System (static)")
set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS system)
include_directories (${Boost_INCLUDE_DIR})
message ("-- Boost binary: " ${Boost_LIBRARIES})

thread_packages_check()

include_directories (${CMAKE_SOURCE_DIR})

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
    src/LicenseManager.cc
    src/License.cc
    src/IssuingAuthority.cc
)

set (SHARED_REQUIRED_LIBS
    ${RIPE_LIBRARY}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

############# RESIDUE CORE LIB (STATIC) ###############

add_library (residue-core STATIC ${SOURCE_FILES})

set_target_properties (residue-core PROPERTIES
    VERSION ${RESIDUE_VERSION}
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
#    tools/config-validator/main.cc
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


############# LICENSE_MANAGER TOOL ###############

add_executable (residue-license-manager-bin
    tools/license-manager/main.cc
    ${EASYLOGGINGPP_INCLUDE_DIR}/easylogging++.cc
)

target_compile_definitions (residue-license-manager-bin PUBLIC
    ELPP_NO_DEFAULT_LOG_FILE
    ELPP_NO_LOG_TO_FILE
    ELPP_STL_LOGGING
    AUTO_INITIALIZE_EASYLOGGINGPP
    ELPP_DEFAULT_LOGGING_FLAGS=4096
)

target_link_libraries (residue-license-manager-bin ${RIPE_LIBRARY} residue-core)

set_target_properties (residue-license-manager-bin PROPERTIES
    OUTPUT_NAME "residue-license-manager"
    VERSION ${RESIDUE_VERSION}
)

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
