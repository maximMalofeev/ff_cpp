#pragma once
#include <ff_cpp/ff_include.h>

namespace ff_cpp {

class Decoder {
 public:
  /**
   * @brief Decoder constructor
   *
   * @param codecId
   * @param codecpar
   * @param userParams
   * @exception NoDecoder if unable to find decoder for required codec id or
   * unable to open codec
   * @exception FFCppException if unable to alloc decoder context or codecpar
   * not copied to decoder context
   * @exception OptionsNotAccepted if not all params accepted
   * @return FF_CPP_API
   */
  FF_CPP_API explicit Decoder(AVCodecID codecId, AVCodecParameters* codecpar = nullptr,
                     const ParametersContainer& userParams = {});
  FF_CPP_API Decoder(Decoder&&);
  FF_CPP_API ~Decoder();

  FF_CPP_API AVMediaType mediaType() const;
  FF_CPP_API AVCodecID codec() const;
  FF_CPP_API int width() const;
  FF_CPP_API int height() const;
  FF_CPP_API int format() const;

  FF_CPP_API int sendPacket(const AVPacket* pkt) const;
  FF_CPP_API int receiveFrame(AVFrame* frame);

  FF_CPP_API friend std::ostream& operator<<(std::ostream& ost,
                                             const Decoder& dcdr);

 private:
  Decoder(const Decoder&) = delete;
  Decoder& operator=(const Decoder&) = delete;
  Decoder&& operator=(const Decoder&&) = delete;

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace ff_cpp