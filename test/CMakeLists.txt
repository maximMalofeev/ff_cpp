project(ff_cpp_test)

add_executable(demuxer_tst ff_demuxer_tst.cpp)
target_link_libraries(demuxer_tst PRIVATE ff_cpp CONAN_PKG::catch2)
target_compile_options(demuxer_tst PRIVATE /std:c++17)

if(WIN32)
  set_target_properties(demuxer_tst PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

include(CTest)
include(Catch)
catch_discover_tests(demuxer_tst WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test/assets")