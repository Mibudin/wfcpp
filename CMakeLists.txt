cmake_minimum_required(VERSION 3.15)

project(wfcpp_project
    VERSION 1.0
    LANGUAGES CXX
    DESCRIPTION
    "WFCpp is a simple library for Wave Function Collapse implemented with \
modern C++ and also provides interfaces to Python with pybind11."
)


find_package(pybind11 REQUIRED)
find_package(Python3 COMPONENTS NumPy REQUIRED)


if (NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))
    set(CMAKE_BUILD_TYPE "Release"
        CACHE STRING "Choose the type of build." FORCE
    )
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS "Release" "RelWithDebInfo" "MinSizeRel" "Debug"
    )
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


add_subdirectory("${PROJECT_SOURCE_DIR}/core")
