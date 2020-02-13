project(ff_cpp_test)

add_executable(demuxer_tst ff_demuxer_tst.cpp)
target_link_libraries(demuxer_tst PRIVATE ff_cpp CONAN_PKG::catch2)

if(WIN32)
  set_target_properties(demuxer_tst PROPERTIES LINK_FLAGS "/ignore:4099")
endif()
