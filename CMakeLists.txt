cmake_minimum_required(VERSION 3.16)
project(delta LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type specified, forcing Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(ExternalProject)

# ---- Executable ----
file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/runtime)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# ---- Libraries ----
# spdlog
add_subdirectory(vendor/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# DIA SDK
set(DIA_SDK_DIR "C:/Program Files/Microsoft Visual Studio/2022/Community/DIA SDK")
link_directories("${DIA_SDK_DIR}/lib/amd64")
include_directories("${DIA_SDK_DIR}/include")

# LLVM
set(LLVM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/llvm")
set(CMAKE_PREFIX_PATH ${LLVM_DIR})
find_package(LLVM REQUIRED CONFIG)

message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "LLVM_VERSION: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")

target_include_directories(${PROJECT_NAME} PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LLVM_LIBS})
llvm_map_components_to_libnames(LLVM_LIBS core orcjit native)