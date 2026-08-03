cmake_minimum_required(VERSION 3.20)
project(pyfbx LANGUAGES C CXX)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

add_library(ufbx STATIC third_party/ufbx/ufbx.c)
target_include_directories(ufbx PUBLIC third_party/ufbx)
set_target_properties(ufbx PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    POSITION_INDEPENDENT_CODE YES
)

if(MSVC)
    target_compile_options(ufbx PRIVATE /W3 /wd4244 /wd4267)
else()
    target_compile_options(ufbx PRIVATE -w)
endif()

pybind11_add_module(_native MODULE
    src/native/module.cpp
    src/native/common.cpp
    src/native/scene.cpp
    src/native/mesh.cpp
    src/native/material.cpp
)
target_link_libraries(_native PRIVATE ufbx)
target_include_directories(_native PRIVATE src/native)
target_compile_features(_native PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(_native PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(_native PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Wheels must not depend on DLLs from the build machine's MSYS2 install.
    target_link_options(_native PRIVATE -static-libgcc -static-libstdc++)
    target_link_libraries(_native PRIVATE -Wl,-Bstatic winpthread -Wl,-Bdynamic)
endif()

install(TARGETS _native LIBRARY DESTINATION pyfbx RUNTIME DESTINATION pyfbx)
