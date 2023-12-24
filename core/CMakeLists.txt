cmake_minimum_required(VERSION 3.15)

project(wfcpp
    VERSION 1.0
    LANGUAGES CXX
)


set(${PROJECT_NAME}_SOURCE
    "${PROJECT_SOURCE_DIR}/src/propagator.cpp"
    "${PROJECT_SOURCE_DIR}/src/wave.cpp"
    "${PROJECT_SOURCE_DIR}/src/wfc_core.cpp"
)

set(${PROJECT_NAME}_HEADER
    "${PROJECT_SOURCE_DIR}/inc/utils/allocator.hpp"
    "${PROJECT_SOURCE_DIR}/inc/utils/array.hpp"
    "${PROJECT_SOURCE_DIR}/inc/utils/color.hpp"
    "${PROJECT_SOURCE_DIR}/inc/overlapping_model.hpp"
    "${PROJECT_SOURCE_DIR}/inc/propagator.hpp"
    "${PROJECT_SOURCE_DIR}/inc/wave.hpp"
    "${PROJECT_SOURCE_DIR}/inc/wfc_core.hpp"
    "${PROJECT_SOURCE_DIR}/inc/wfc_def.hpp"
)

set(${PROJECT_NAME}_PYBIND11_MOD
    "${PROJECT_SOURCE_DIR}/src/mod.pybind11.cpp"
)


# A wrapper function around `add_library` provided by pybind11
pybind11_add_module(${PROJECT_NAME} MODULE
    ${${PROJECT_NAME}_SOURCE}
    ${${PROJECT_NAME}_HEADER}
    ${${PROJECT_NAME}_PYBIND11_MOD}
)


target_include_directories(${PROJECT_NAME}
    PRIVATE "${PROJECT_SOURCE_DIR}/inc"
    PRIVATE "${PYTHON_INCLUDE_DIRS}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(${PROJECT_NAME}
    # PRIVATE -O2  # Already set by CMake with building configuration
    # PRIVATE -W
    PRIVATE -Wall -Wextra
    PRIVATE -Wno-reorder  # Too annoying
)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND stubgen -m ${PROJECT_NAME} -o .
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT
    "Use stubgen to create `${PROJECT_NAME}.pyi` for statement completion"
)


install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Debug Release
    LIBRARY DESTINATION lib
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pyi"
    CONFIGURATIONS Debug Release
    TYPE LIB
)
