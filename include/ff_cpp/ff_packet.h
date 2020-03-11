#pragma once
#include <ff_cpp/ff_include.h>

namespace ff_cpp {

class Packet {
 public:
  FF_CPP_API Packet();
  FF_CPP_API ~Packet();

  /**
   * @brief Return packet's presentation time stamp
   * 
   * @return pts 
   */
  FF_CPP_API int64_t pts() const;
  /**
   * @brief Return packet's decoding time stamp
   * 
   * @return FF_CPP_API dts 
   */
  FF_CPP_API int64_t dts() const;
  /**
   * @brief Return corresponding stream index
   * 
   * @return FF_CPP_API streamIndex 
   */
  FF_CPP_API int streamIndex() const;

  friend std::ostream& operator<<(std::ostream& ost, const Packet& pkt);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

  friend class Demuxer;
  friend class Decoder;
  operator AVPacket*();
};

}  // namespace ff_cpp
