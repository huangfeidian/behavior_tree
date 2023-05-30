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
set(BTREE_COMMON_INCLUDE_DIR ${BTREE_COMMON_DIR}/include/)

INCLUDE_DIRECTORIES(${BTREE_COMMON_INCLUDE_DIR})


file(GLOB SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")


# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

find_package(any_container CONFIG REQUIRED)

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

add_library(behavior_tree_runtime  ${SRC_FILES})

target_link_libraries(behavior_tree_runtime PUBLIC nlohmann_json::nlohmann_json any_container)

add_executable(debug_test ${PROJECT_SOURCE_DIR}/test/debug_test.cpp)
target_link_libraries(debug_test behavior_tree_runtime)

foreach(p LIB BIN INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		string(TOLOWER ${p} lower_p)
        set(${var} "${CMAKE_INSTALL_PREFIX}/${lower_p}")
	endif()
endforeach()

export(TARGETS ${CMAKE_PROJECT_NAME}
     FILE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)

file(WRITE
    ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
    "include(\${CMAKE_CURRENT_LIST_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)\n")

install(FILES
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
   ARCHIVE  DESTINATION lib
   LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ ${BTREE_COMMON_INCLUDE_DIR} DESTINATION include/${PROJECT_NAME})

install(EXPORT ${CMAKE_PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})
