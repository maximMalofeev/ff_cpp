cmake_minimum_required(VERSION 3.10)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(example example.cpp)
target_link_libraries(example PUBLIC CONAN_PKG::ff_cpp)

add_test(NAME example WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin COMMAND example)
