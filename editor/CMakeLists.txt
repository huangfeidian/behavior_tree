cmake_minimum_required(VERSION 3.5)

project(behavior_tree_editor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

set(BTREE_COMMON_DIR ${PROJECT_SOURCE_DIR}/../common)
set(BTREE_COMMON_INCLUDE_DIR ${BTREE_COMMON_DIR}/include)
set(BTREE_COMMON_SRC_DIR ${BTREE_COMMON_DIR}/src)

INCLUDE_DIRECTORIES(${BTREE_COMMON_INCLUDE_DIR})

file(GLOB_RECURSE EDITOR_SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE EDITOR_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/*.h")







# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

find_package(any_container CONFIG REQUIRED)

find_package(tree_editor CONFIG REQUIRED)
file(GLOB_RECURSE TREE_EDITOR_HEADER "${tree_editor_include_dirs}/tree_editor/*.h")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${tree_editor_include_dirs}/tree_editor/resource/)

file(GLOB_RECURSE UI_FILES "${tree_editor_include_dirs}/tree_editor/resource/*.ui")

set(AUTOGEN_BUILD_DIR ${PROJECT_SOURCE_DIR})
find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(behavior_tree_editor ${UI_FILES}  ${TREE_EDITOR_HEADER} ${EDITOR_SRC_FILES} ${EDITOR_HEADER_FILES})



target_link_libraries(behavior_tree_editor PUBLIC tree_editor::tree_editor Qt5::Widgets nlohmann_json::nlohmann_json any_container)
