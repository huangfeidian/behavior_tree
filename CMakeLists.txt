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


add_subdirectory(runtime)

if(WITH_GUI)
add_subdirectory(editor)
add_subdirectory(debugger)
endif(WITH_GUI)
