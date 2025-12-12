cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PktParser)

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

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CASSANDRA_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
	fmt::fmt
	nlohmann_json::nlohmann_json
	${CASSANDRA_LIBRARY}
	${ZLIB_LIBRARIES}
)
