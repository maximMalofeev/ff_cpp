project(ff_player)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ff_cpp CONAN_PKG::sdl2)

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
elseif(UNIX)
endif()