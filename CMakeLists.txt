cmake_minimum_required(VERSION 3.2)
project(marketmaker-cli)

## use or not gcov
option(ENABLE_GCOV "Enable gcov" OFF)

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

configure_file(version.h.in include/version.h @ONLY)
configure_file(sonar-project.properties.in sonar-project.properties @ONLY)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    set(CXX_WARNINGS -Werror -Wall -Wno-sign-compare -Wmissing-declarations -Wcast-qual -Wwrite-strings)
    set(C_WARNINGS ${CXX_WARNINGS} -Wstrict-prototypes -Wmissing-prototypes)
    set(GCOV_CC_FLAGS --coverage -g -O0)
    set(GCOV_LD_FLAGS --coverage)
endif ()

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

set(CFLAGS_LIST ${CMAKE_C_FLAGS})
separate_arguments(CFLAGS_LIST)

set(SOURCE_FILES
        src/main/mmcli.c
        src/main/basic.h
        src/main/safe_alloc.c
        src/main/safe_alloc.h
        src/main/property.c
        src/main/property.h
        src/main/strutil.c
        src/main/strutil.h
        src/main/api_help.c
        src/main/api_help.h
        src/main/json.c
        src/main/json.h
        src/main/url.c
        src/main/url.h
        src/main/sys_socket.c
        src/main/socket.h
        src/main/sys_socket.h
        src/main/http.c
        src/main/http.h
        src/main/path.c
        src/main/path.h
        src/main/file.h
        src/main/sys_file.c
        src/main/sys_file.h
        )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PUBLIC ${C_WARNINGS})
if(WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

include(CTest)
set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

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

include_directories("src/main" ${CMAKE_BINARY_DIR}/include)

add_executable(tests
        src/test/url_tests.cc
        src/test/property_tests.cc
        src/main/basic.h
        src/main/safe_alloc.c
        src/main/safe_alloc.h
        src/main/property.c
        src/main/strutil.c
        src/main/http.c
        src/main/url.c
        src/main/api_help.c
        src/test/api_help_tests.cc
        src/main/json.c
        src/test/json_tests.cc
        src/test/http_tests.cc
        src/test/CppSocket.h
        src/test/CppSocket.cc
        src/test/MockSocket.h
        src/main/file.h
        src/test/CppFile.cc
        src/test/CppFile.h
        src/test/MockFile.h
        src/test/main.cc)
target_link_libraries(tests gtest gmock)
if(WIN32)
    target_link_libraries(tests wsock32 ws2_32)
endif()

if(ENABLE_GCOV)
    target_compile_options(tests PUBLIC $<$<COMPILE_LANGUAGE:C>:${GCOV_CC_FLAGS}>)
    set_target_properties(tests PROPERTIES LINK_FLAGS ${GCOV_LD_FLAGS})
endif(ENABLE_GCOV)

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

