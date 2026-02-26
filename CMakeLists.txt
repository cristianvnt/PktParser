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
find_package(libuv CONFIG REQUIRED)

if (WIN32)
	set(CASSANDRA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/deps/cassandra-cpp-driver)
	
	set(CASSANDRA_LIB ${CASSANDRA_ROOT}/lib/cassandra.lib)
	set(CASSANDRA_DLL ${CASSANDRA_ROOT}/bin/cassandra.dll)

	if (NOT EXISTS "${CASSANDRA_LIB}" OR NOT EXISTS "${CASSANDRA_DLL}")
		message(FATAL_ERROR "Cassandra library files not found in ${CASSANDRA_ROOT}")
	endif()

	add_library(Cassandra::Cassandra SHARED IMPORTED GLOBAL)
	set_target_properties(Cassandra::Cassandra PROPERTIES
		IMPORTED_LOCATION ${CASSANDRA_DLL}
		IMPORTED_IMPLIB ${CASSANDRA_LIB}
		INTERFACE_INCLUDE_DIRECTORIES ${CASSANDRA_ROOT}/include
	)
else()
    message(FATAL_ERROR "NYI")
endif()

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
	$<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
	OpenSSL::SSL
	OpenSSL::Crypto
	Cassandra::Cassandra
)

if (Drogon_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
endif()

if (WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CASSANDRA_DLL}
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
	)
endif()

if (NOT WIN32 AND JEMALLOC_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIB})
endif()
