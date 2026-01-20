cmake_minimum_required(VERSION 3.28)
cmake_policy(SET CMP0135 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PktParser)

# Parallel parsing by default
option(SYNC_PARSING "Enable single-threaded parsing" OFF)

if(SYNC_PARSING)
	add_compile_definitions(SYNC_PARSING_MODE)	
    message(STATUS "Building in SYNC mode")
else()
	add_compile_definitions(PARALLEL_PARSING_MODE)
	message(STATUS "Building in PARALLEL mode")
endif()

# ccache
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache")
endif()

# dependencies
include(FetchContent)

FetchContent_Declare(json
	URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

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

# include dirs
target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/PCH
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser/Versions
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser/Common
	${CASSANDRA_INCLUDE_DIR}
)

# libs
target_link_libraries(${PROJECT_NAME} PRIVATE 
	fmt::fmt
	nlohmann_json::nlohmann_json
	pqxx
	${CASSANDRA_LIBRARY}
	${ZLIB_LIBRARIES}
	Threads::Threads
)
