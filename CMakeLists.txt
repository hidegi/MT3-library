cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(mot VERSION 1.1.1 LANGUAGES C CXX)

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

if(NOT SP_PARENT_DIRECTORY AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "choose a build type: Debug|Release|RelWithDebInfo|MinSizeRel" FORCE)
endif()
message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

set(SP_VERSION_MAJOR ${playground_VERSION_MAJOR})
set(SP_VERSION_MINOR ${playground_VERSION_MINOR})
set(SP_VERSION_PATCH ${playground_VERSION_PATCH}) 
set(SP_VERSION_FULL "${SP_VERSION_MAJOR}.${SP_VERSION_MINOR}.${SP_VERSION_PATCH}")
set(SP_VERSION ${playground_VERSION})
set(SP_PARENT_DIRECTORY PARENT_DIRECTORY)
set(SP_LIB_NAME mot)

add_definitions(-DGLEW_STATIC)

if(NOT BUILD_SHARED_LIBS)
	set(SP_STATIC_BUILD 1)
endif()

set(SP_LIB_NAME_SUFFIX "")
if(WIN32)
	set(SP_PLATFORM_WINDOWS 1)
	if(BUILD_SHARED_LIBS)
		set(SP_LIB_NAME_SUFFIX "dll")
	endif()
endif()

if(APPLE)
	set(SP_PLATFORM_MACOS 1)
endif()

sp_set_option(SP_OUTPUT_DEBUG OFF BOOL "Print the read and write procedures")

if(SP_OUTPUT_DEBUG)
	set(MOT_PRINT_OUTPUT_DEBUG 1)
endif()

configure_file(
	"${CMAKE_SOURCE_DIR}/include/SP/config.h.in"
	"${CMAKE_SOURCE_DIR}/include/SP/config.h" ESCAPE_QUOTES)

sp_set_option(BUILD_SHARED_LIBS OFF BOOL "Build shared libraries")
sp_set_option(SP_BUILD_EXAMPLES ON BOOL "Build the MOT example programs")
sp_set_option(SP_BUILD_TESTS ON BOOL "Build tests")




include_directories(${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests)

add_subdirectory(src)

if(SP_BUILD_TESTS)
	add_subdirectory(tests)
endif()

find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)
#if(WIN32)
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES} winmm)
#elseif(APPLE)
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES})
#elseif(UNIX AND NOT APPLE)
#	target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} ${ZLIB_LIBRARIES})
#endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBRARIES})
set_target_properties(
	${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	VERSION ${plaground_VERSION}
	SOVERSION ${plaground_VERSION}
	)
	