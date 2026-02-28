cmake_minimum_required(VERSION 3.28)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(PktParser)

# ccache
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache")
endif()

# dependencies
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
)
FetchContent_MakeAvailable(cassandra-cpp-driver)
FetchContent_GetProperties(cassandra-cpp-driver SOURCE_DIR CASS_SOURCE_DIR)
target_include_directories(cassandra_static INTERFACE ${CASS_SOURCE_DIR}/include)
add_compile_definitions(CASS_STATIC)

# vcpkg dependencies
find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(zstd REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(libpqxx REQUIRED)

find_package(Drogon)
if (Drogon_FOUND)
	message(STATUS "Drogon found - API server mode enabled")
	add_compile_definitions(HAS_DROGON)
else()
	message(STATUS "Drogon not found - API server mode disabled")
endif()

find_library(JEMALLOC_LIB jemalloc)
if(JEMALLOC_LIB)
    message(STATUS "Found jemalloc: ${JEMALLOC_LIB}")
endif()

# PCH
set(PCH_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/PCH/pchdef.h")
set(PCH_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/PCH/pchdef.cpp")

# source files
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${PCH_SOURCE} ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH_HEADER})

# compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g3 -Wall -Wextra)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g -Wall -Wextra -flto=auto)
	target_link_options(${PROJECT_NAME} PRIVATE -flto)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native -flto)
    target_link_options(${PROJECT_NAME} PRIVATE -flto)
endif()

find_library(JEMALLOC_LIB jemalloc)
if(JEMALLOC_LIB)
    message(STATUS "Found jemalloc: ${JEMALLOC_LIB}")
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
    zstd::libzstd_static
    OpenSSL::SSL
    OpenSSL::Crypto
    cassandra_static
)

if(Drogon_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
endif()

if(JEMALLOC_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIB})
endif()
