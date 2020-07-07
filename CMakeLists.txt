cmake_minimum_required(VERSION 3.10)

project(ff_cpp)

set(${PROJECT_NAME}_VERSION_MAJOR 0)
set(${PROJECT_NAME}_VERSION_MINOR 1)

if(CONAN_EXPORTED)
  include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  conan_basic_setup(TARGETS)
else()
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
     message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
     file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
                   "${CMAKE_BINARY_DIR}/conan.cmake")
  endif()
  include("${CMAKE_BINARY_DIR}/conan.cmake")

  conan_add_remote(NAME conan-center INDEX 1 URL "https://conan.bintray.com")
  conan_add_remote(NAME bincrafters INDEX 2 URL "https://api.bintray.com/conan/bincrafters/public-conan")

  conan_cmake_run(
    CONANFILE conanfile.py
    BASIC_SETUP CMAKE_TARGETS
    BUILD missing
    ENV CC="${CMAKE_C_COMPILER}"
    ENV CXX="${CMAKE_CXX_COMPILER}")
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(sources "include/ff_cpp/ff_include.h" "include/ff_cpp/ff_exception.h"
  "include/ff_cpp/ff_info.h" "src/ff_info.cpp"
  "include/ff_cpp/ff_demuxer.h" "src/ff_demuxer.cpp"
  "include/ff_cpp/ff_stream.h" "src/ff_stream.cpp"
  "include/ff_cpp/ff_decoder.h" "src/ff_decoder.cpp"
  "include/ff_cpp/ff_filter.h" "src/ff_filter.cpp"
  "include/ff_cpp/ff_packet.h" "src/ff_packet.cpp"
  "include/ff_cpp/ff_frame.h" "src/ff_frame.cpp"
  "include/ff_cpp/ff_scaler.h" "src/ff_scaler.cpp")

add_library(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC CONAN_PKG::ffmpeg)

if(WIN32)
  set(FF_CPP_DEFINES -DFF_CPP)
  if(${BUILD_SHARED_LIBS})
    set(FF_CPP_DEFINES ${FF_CPP_DEFINES} -DFF_CPP_SHARED)
  endif()
  target_compile_definitions(${PROJECT_NAME} PRIVATE ${FF_CPP_DEFINES})
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
elseif(UNIX)
endif()

enable_testing()
add_subdirectory(test)
add_subdirectory(examples)

