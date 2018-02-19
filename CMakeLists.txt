cmake_minimum_required(VERSION 3.2)
project(marketmaker-cli)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/;${CMAKE_MODULE_PATH}")

include(cmake/GetGitRevisionDescription.cmake)
git_describe(VERSION "--dirty")
# parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(PACKAGE_RELEASE "1")    # fixed
message(STATUS "Version set to: " ${VERSION})

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    set(CXX_WARNINGS -Werror -Wall -Wno-sign-compare -Wmissing-declarations -Wcast-qual -Wwrite-strings)
    set(CC_WARNINGS ${CXX_WARNINGS} -Wstrict-prototypes -Wmissing-prototypes)
    set(CC_OPTIONS -fdata-sections -ffunction-sections)
    set(LD_OPTIONS -Wl,--gc-sections)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
else ()
    message(FATAL_ERROR "Unsupported C compiler ${CMAKE_C_COMPILER_ID}")
endif ()

set(CFLAGS_LIST ${CMAKE_C_FLAGS})
separate_arguments(CFLAGS_LIST)

set(SOURCE_FILES
        src/main/mmcli.c
        src/main/property.c
        src/main/strutil.c
        src/main/api_help.c
        src/main/api_help.h
        src/main/json.c
        src/main/json.h
        src/main/http.c
        src/main/http.h src/main/path.c src/main/path.h)

add_executable(marketmaker-cli ${SOURCE_FILES})

install(TARGETS marketmaker-cli RUNTIME DESTINATION bin)

include(CTest)
if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else ()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif ()

include(DownloadProject.cmake)
download_project(PROJ googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.0
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
endif ()

include_directories("src/main")

add_executable(tests
        src/test/property_tests.cc
        src/main/property.c
        src/main/strutil.c
        src/main/http.c
        src/main/api_help.c
        src/test/api_help_tests.cc
        src/test/property_utils.cc
        src/main/json.c
        )
target_link_libraries(tests gtest gmock_main)

add_test(NAME Tests COMMAND tests)

set(CPACK_SET_DESTDIR ON)

set(CPACK_PACKAGE_NAME "marketmaker-cli")
set(CPACK_PACKAGE_VENDOR "The SuperNET Developers")
set(CPACK_PACKAGE_CONTACT "Eliezio Oliveira <eliezio@protonmail.com>")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_RPM_PACKAGE_RELEASE ${PACKAGE_RELEASE})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MarketMaker Command-Line Interface")
SET(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${VERSION_SHORT})
set(CPACK_RPM_SPEC_INSTALL_POST "/bin/true")
include(CPack)

