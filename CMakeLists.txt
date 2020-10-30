
cmake_minimum_required(VERSION 3.16)
include(FetchContent)
set(CMAKE_CXX_COMPILER g++-10)
set(CMAKE_C_COMPILER   gcc-10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG -funroll-loops")
project(multiarray)
FetchContent_Declare(
    cppavx
    GIT_REPOSITORY https://github.com/manuel5975p/cpp-avx-utils.git
)
FetchContent_GetProperties(cppavx)
if(NOT cppavx_POPULATED)
    FetchContent_Populate(cppavx)
endif()
add_library(multi_array INTERFACE)
target_include_directories(multi_array INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(multi_array INTERFACE "${cppavx_SOURCE_DIR}")
add_executable(test "test.cpp")
target_link_libraries(test multi_array)