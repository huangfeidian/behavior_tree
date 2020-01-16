cmake_minimum_required(VERSION 3.5)

project(behavior_tree_runtime LANGUAGES CXX)

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


file(GLOB SRC_FILES "${PROJECT_SOURCE_DIR}/include/*.cpp")


# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

find_package(any_container CONFIG REQUIRED)

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

add_executable(behavior_tree_runtime ${COMMON_SRC} ${SRC_FILES})

target_link_libraries(behavior_tree_runtime PUBLIC nlohmann_json::nlohmann_json any_container)
