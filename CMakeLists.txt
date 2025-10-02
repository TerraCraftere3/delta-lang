cmake_minimum_required(VERSION 3.16)
project(delta LANGUAGES CXX C)

set(DELTA_COMPILER_NAME "Official Delta Compiler")
set(DELTA_COMPILER_VERSION "2025.0.1")
set(DELTA_STDLIB_VERSION "2025.0")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type specified, forcing Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(ExternalProject)

# ---- Standard Library ----
# Collect all C source files from stdlib directory
file(GLOB STDLIB_C_FILES CONFIGURE_DEPENDS stdlib/*.c)
file(GLOB STDLIB_H_FILES CONFIGURE_DEPENDS stdlib/*.h)

# Create static library from stdlib files
if(STDLIB_C_FILES)
    add_library(deltaStdLib STATIC ${STDLIB_C_FILES})
    target_include_directories(deltaStdLib PUBLIC stdlib)
    
    # Set C standard for the stdlib
    set_property(TARGET deltaStdLib PROPERTY C_STANDARD 11)
    set_property(TARGET deltaStdLib PROPERTY C_STANDARD_REQUIRED ON)
    
    # Ensure we get both debug and release versions
    set_target_properties(deltaStdLib PROPERTIES
        OUTPUT_NAME "stdlib"
    )
    
    # Platform-specific linking for graphics/windowing
    if(WIN32)
        target_link_libraries(deltaStdLib PRIVATE gdi32 user32 kernel32)
    elseif(UNIX AND NOT APPLE)
        # Linux - link X11
        find_package(X11)
        if(X11_FOUND)
            target_link_libraries(deltaStdLib PRIVATE ${X11_LIBRARIES})
            target_include_directories(deltaStdLib PRIVATE ${X11_INCLUDE_DIR})
        endif()
    elseif(APPLE)
        # macOS - link Cocoa framework
        find_library(COCOA_LIBRARY Cocoa)
        if(COCOA_LIBRARY)
            target_link_libraries(deltaStdLib PRIVATE ${COCOA_LIBRARY})
        endif()
    endif()
    
    # Set output directory for the library
    set_target_properties(deltaStdLib PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    )
else()
    message(WARNING "No C files found in stdlib directory")
endif()

# ---- Executable ----
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/Wasm.html" WASM_HTML_CONTENT)
set(WASM_HTML "${WASM_HTML_CONTENT}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Wasm.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Wasm.h"
    @ONLY
)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Wasm.html"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Globals.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Globals.h"
    @ONLY
)

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/runtime)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# ---- Libraries ----
# spdlog
add_subdirectory(vendor/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)