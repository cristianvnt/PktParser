cmake_minimum_required(VERSION 3.28)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PktParser)

# ccache
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache")
endif()

# dependencies
include(FetchContent)

set(BUILD_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_TEST OFF CACHE BOOL "" FORCE)
set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)

FetchContent_Declare(libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.10.4
)
FetchContent_MakeAvailable(libpqxx)

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZSTD REQUIRED libzstd)
find_package(OpenSSL REQUIRED)

list(APPEND CMAKE_PREFIX_PATH /usr/local)
find_package(Drogon)
if (Drogon_FOUND)
	message(STATUS "Drogon found - API server mode enabled")
	add_compile_definitions(HAS_DROGON)
else()
	message(STATUS "Drogon not found - API server mode disabled")
endif()

find_library(CASSANDRA_LIBRARY NAMES cassandra REQUIRED)
find_path(CASSANDRA_INCLUDE_DIR NAMES cassandra.h REQUIRED)

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
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g -Wall -Wextra)
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
	${CASSANDRA_INCLUDE_DIR}
	${ZSTD_INCLUDE_DIRS}
)

# libs
target_link_libraries(${PROJECT_NAME} PRIVATE 
	fmt::fmt
	pqxx
	${CASSANDRA_LIBRARY}
	${ZLIB_LIBRARIES}
	Threads::Threads
	CURL::libcurl
	${ZSTD_LIBRARIES}
	OpenSSL::Crypto
)

if(Drogon_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
endif()

if(JEMALLOC_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIB})
endif()
