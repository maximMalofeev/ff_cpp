cmake_minimum_required(VERSION 3.10)

project(ff_cpp)

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.8.0 REQUIRED)
conan_add_remote(NAME bincrafters URL "https://api.bintray.com/conan/bincrafters/public-conan")
conan_add_remote(NAME conan-center URL "https://conan.bintray.com")

conan_cmake_run(CONANFILE conanfile.py
                BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
                INSTALL_FOLDER ${CMAKE_BINARY_DIR}
                BUILD missing)

set(sources "include/ff_cpp/ff_include.h" "include/ff_cpp/ff_exception.h"
  "include/ff_cpp/ff_info.h" "src/ff_info.cpp"
  "include/ff_cpp/ff_demuxer.h" "src/ff_demuxer.cpp"
  "include/ff_cpp/ff_stream.h" "src/ff_stream.cpp"
  "include/ff_cpp/ff_decoder.h" "src/ff_decoder.cpp")

add_library(${PROJECT_NAME} SHARED ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC CONAN_PKG::ffmpeg)

if(WIN32)
  add_compile_options(-MP$ENV{NUMBER_OF_PROCESSORS})
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DFF_CPP)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
elseif(UNIX)
endif()

add_subdirectory(test)
#add_subdirectory(examples)

