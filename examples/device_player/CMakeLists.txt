project(device_player)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ff_cpp CONAN_PKG::sdl2)
target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17)

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
endif()