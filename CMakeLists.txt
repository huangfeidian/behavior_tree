cmake_minimum_required(VERSION 3.5)

project(behavior_tree LANGUAGES CXX)

option(WITH_TEST "with test" ON)
option(WITH_GUI "with gui" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

find_package(any_container CONFIG REQUIRED)

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

add_subdirectory(runtime)

if(WITH_GUI)
add_subdirectory(editor)
add_subdirectory(debugger)
endif(WITH_GUI)
