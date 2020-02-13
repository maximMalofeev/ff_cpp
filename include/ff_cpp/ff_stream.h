#pragma once
#include <ff_cpp/ff_include.h>

namespace ff_cpp {

class Stream {
 public:
  FF_CPP_API Stream(Stream&&);
  FF_CPP_API ~Stream();

  /**
   * @brief the stream index in FFDemuxer
   *
   * @return int
   */
  FF_CPP_API int index() const;

  /**
   * @brief stream media type
   *
   * @return AVMediaType or AVMEDIA_TYPE_UNKNOWN if there is no stream
   */
  FF_CPP_API AVMediaType mediaType() const;

  /**
   * @brief stream codec id
   *
   * @return AVCodecID or AV_CODEC_ID_NONE if there is no stream
   */
  FF_CPP_API AVCodecID codec() const;

  FF_CPP_API int width() const;
  FF_CPP_API int height() const;
  FF_CPP_API int format() const;
  FF_CPP_API int averageFPS() const;
  FF_CPP_API AVRational timeBase() const;

  friend class Demuxer;
  FF_CPP_API friend std::ostream& operator<<(std::ostream& ost, const Stream& s);

 private:
  Stream(AVStream* stream);
  Stream(const Stream&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream&& operator=(const Stream&&) = delete;

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace ff_cpp