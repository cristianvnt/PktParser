cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

if (DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(PktParser)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# ccache
if (NOT WIN32)
	find_program(CCACHE_PROGRAM ccache)
	if (CCACHE_PROGRAM)
		set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		message(STATUS "Using ccache")
	endif()
endif()

# vcpkg dependencies
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(zstd REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libpqxx REQUIRED)

# windows cassandra shenanigans
include(FetchContent)
set(CASS_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(CASS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
set(CASS_USE_OPENSSL ON CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

FetchContent_Declare(cassandra-cpp-driver
    GIT_REPOSITORY https://github.com/apache/cassandra-cpp-driver.git
    GIT_TAG 2.17.1
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_SOURCE_DIR}/cmake/patch_cassandra_msvc.cmake
)
FetchContent_MakeAvailable(cassandra-cpp-driver)
FetchContent_GetProperties(cassandra-cpp-driver SOURCE_DIR CASS_SOURCE_DIR)
target_include_directories(cassandra_static INTERFACE ${CASS_SOURCE_DIR}/include)

# optional
find_package(Drogon CONFIG)
if (Drogon_FOUND)
	message(STATUS "Drogon found - API server mode enabled")
	add_compile_definitions(HAS_DROGON)
else()
	message(STATUS "Drogon not found - API server mode disabled")
endif()

if (NOT WIN32)
	find_library(JEMALLOC_LIB jemalloc)
	if (JEMALLOC_LIB)
		message(STATUS "Found jemalloc: ${JEMALLOC_LIB}")
	endif()
endif()

# PCH
set(PCH_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/PCH/pchdef.h")
set(PCH_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/PCH/pchdef.cpp")

# src files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${PCH_SOURCE} ${SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH_HEADER})

target_compile_definitions(${PROJECT_NAME} PRIVATE CASS_STATIC)

# compiler flags
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MP /utf-8)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE /Od /Zi)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /Zi)
        target_link_options(${PROJECT_NAME} PRIVATE /DEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /GL)
        target_link_options(${PROJECT_NAME} PRIVATE /LTCG)
	endif()
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g3)
	elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g -flto=auto)
		target_link_options(${PROJECT_NAME} PRIVATE -flto)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native -flto)
		target_link_options(${PROJECT_NAME} PRIVATE -flto)
	endif()
endif()

# include dirs
target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/PCH
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser/Versions
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser/Common
)

# libs
target_link_libraries(${PROJECT_NAME} PRIVATE 
	fmt::fmt
	libpqxx::pqxx
	ZLIB::ZLIB
	Threads::Threads
	CURL::libcurl
	zstd::libzstd_shared
	OpenSSL::SSL
	OpenSSL::Crypto
	cassandra_static
)

if (Drogon_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
endif()

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()

if (NOT WIN32 AND JEMALLOC_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIB})
endif()
