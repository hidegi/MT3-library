cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(mt3 VERSION 2.0.2 LANGUAGES C CXX)

macro(sp_set_option var default type docstring)
	if(NOT DEFINED ${var})
		set(${var} ${default})
	endif()
	set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

if(POLICY CMP0079)
	cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

cmake_policy(SET CMP0042 NEW)
if (NOT CMAKE_VERSION VERSION_LESS 3.9)
	cmake_policy(SET CMP0068 NEW)
endif()

message(STATUS "Installing project to ${CMAKE_INSTALL_PREFIX}")

if(NOT MT3_PARENT_DIRECTORY AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "choose a build type: Debug|Release|RelWithDebInfo|MinSizeRel" FORCE)
endif()
message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

set(MT3_VERSION_MAJOR ${mt3_VERSION_MAJOR})
set(MT3_VERSION_MINOR ${mt3_VERSION_MINOR})
set(MT3_VERSION_PATCH ${mt3_VERSION_PATCH}) 
set(MT3_VERSION_FULL "${MT3_VERSION_MAJOR}.${MT3_VERSION_MINOR}.${MT3_VERSION_PATCH}")
set(MT3_VERSION ${mt3_VERSION_MAJOR}.${mt3_VERSION_MINOR})
set(MT3_PARENT_DIRECTORY PARENT_DIRECTORY)
set(MT3_LIB_NAME mt3)

if(NOT BUILD_SHARED_LIBS)
	set(SP_STATIC_BUILD 1)
endif()

set(MT3_LIB_NAME_SUFFIX "")
if(WIN32)
	set(SP_PLATFORM_WINDOWS 1)
	if(BUILD_SHARED_LIBS)
		set(SP_LIB_NAME_SUFFIX "dll")
	endif()
endif()

if(UNIX AND NOT APPLE)
	set(SP_PLATFORM_LINUX 1)
endif()

if(APPLE)
	set(SP_PLATFORM_MACOS 1)
	set(CMAKE_MACOSX_RPATH 1)
endif()

# Binary Search Tree will search major inclined.
sp_set_option(MT3_HAVE_BST_MAJOR_INCLINED ON BOOL "Have binary search major inclined")

configure_file(
	"${CMAKE_SOURCE_DIR}/include/platform.h.in"
	"${CMAKE_SOURCE_DIR}/include/platform.h" ESCAPE_QUOTES
 )

sp_set_option(BUILD_SHARED_LIBS OFF BOOL "Build shared libraries")
sp_set_option(MT3_BUILD_EXAMPLES ON BOOL "Build the MT3 example programs")
sp_set_option(MT3_BUILD_TESTS ON BOOL "Build tests")

include_directories(
	${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES} 
	${CMAKE_SOURCE_DIR}/include 
	${CMAKE_SOURCE_DIR}/src 
	${CMAKE_SOURCE_DIR}/tests 
	${gtest_SOURCE_DIR}/include
 	${gtest_SOURCE_DIR})

add_library(mt3 src/mt3.c src/buffer.c src/treeops.c src/serialization.c)

if(MT3_BUILD_TESTS)	
	enable_testing()
	add_subdirectory(tests)
endif()

if(MT3_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
find_package(ZLIB REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES})
set_target_properties(
	${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	VERSION ${mt3_VERSION}
	SOVERSION ${mt3_VERSION}
	)
	
