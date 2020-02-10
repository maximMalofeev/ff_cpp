#pragma once
#include <ff_cpp/ff_include.h>

namespace ff_cpp {

class FF_CPP_API Info {
 public:
  static void dumpOutputProtocols();
  static void dumpInputProtocols();
  static void dumpOutputFormats();
  static void dumpInputFormats();
  static void dumpCodecs();
  static void dumpFilters();
  static void dumpConfiguration();
};

}  // namespace ff_cpp