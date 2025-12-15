cmake_minimum_required(VERSION 3.28)
cmake_policy(SET CMP0135 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PktParser)

option(SYNC_PARSING "Enable single-threaded parsing" ON)

if(SYNC_PARSING)
    message(STATUS "Building in SYNC mode")
    add_compile_definitions(SYNC_PARSING_MODE)
else()
    message(STATUS "Building in PARALLEL mode")
    add_compile_definitions(PARALLEL_PARSING_MODE)
endif()

include(FetchContent)

FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(json
	URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

find_library(CASSANDRA_LIBRARY NAMES cassandra REQUIRED)
find_path(CASSANDRA_INCLUDE_DIR NAMES cassandra.h REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g3 -Wall -Wextra -Wpedantic)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -g -Wall -Wextra)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native -flto)
    target_link_options(${PROJECT_NAME} PRIVATE -flto)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parser/Versions
	${CASSANDRA_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
	fmt::fmt
	nlohmann_json::nlohmann_json
	${CASSANDRA_LIBRARY}
	${ZLIB_LIBRARIES}
	Threads::Threads
)
