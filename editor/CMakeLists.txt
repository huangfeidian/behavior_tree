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
file(GLOB_RECURSE COMMON_SRC "${BTREE_COMMON_SRC_DIR}/*.cpp" )

INCLUDE_DIRECTORIES(${BTREE_COMMON_INCLUDE_DIR})

file(GLOB UI_FILES "${PROJECT_SOURCE_DIR}/resource/*.ui")

file(GLOB_RECURSE SHARE_SRC_FILES "${PROJECT_SOURCE_DIR}/src_common/*.cpp")
file(GLOB_RECURSE SHARE_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/*.h")

file(GLOB_RECURSE EDITOR_SRC_FILES "${PROJECT_SOURCE_DIR}/src_editor/*.cpp")
file(GLOB_RECURSE EDITOR_HEADER_FILES "${PROJECT_SOURCE_DIR}/src_editor/*.h")

file(GLOB_RECURSE DEBUGER_SRC_FILES "${PROJECT_SOURCE_DIR}/src_debuger/*.cpp")
file(GLOB_RECURSE DEBUGER_HEADER_FILES "${PROJECT_SOURCE_DIR}/src_debuger/*.h")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/resource/)
set(AUTOGEN_BUILD_DIR ${PROJECT_SOURCE_DIR})
find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)



# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

find_package(any_container CONFIG REQUIRED)

add_executable(behavior_tree_editor ${UI_FILES} ${COMMON_SRC} ${SHARE_SRC_FILES} ${SHARE_HEADER_FILES} ${EDITOR_SRC_FILES} ${EDITOR_SRC_FILES})


target_link_libraries(behavior_tree_editor PRIVATE Qt5::Widgets)
target_link_libraries(behavior_tree_editor PUBLIC nlohmann_json::nlohmann_json any_container)

add_executable(behavior_tree_debuger ${UI_FILES} ${COMMON_SRC} ${SHARE_SRC_FILES} ${SHARE_HEADER_FILES} ${DEBUGER_SRC_FILES} ${DEBUGER_SRC_FILES})
target_link_libraries(behavior_tree_debuger PRIVATE Qt5::Widgets)
target_link_libraries(behavior_tree_debuger PUBLIC nlohmann_json::nlohmann_json any_container)