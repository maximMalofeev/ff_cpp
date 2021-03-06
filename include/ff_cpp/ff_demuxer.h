#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ff_cpp/ff_decoder.h>
#include <ff_cpp/ff_frame.h>
#include <ff_cpp/ff_include.h>
#include <ff_cpp/ff_packet.h>
#include <ff_cpp/ff_stream.h>

namespace ff_cpp {

/**
 * @brief acket_callback will called each time new packed received
 * required return true to decode packet, false to discard it
 * @note return true has a sense only if decoder for appropriate stream created
 */
using packet_callback = std::function<bool(Packet&)>;

/**
 * @brief frame_callback will called each time packet decoded
 */
using frame_callback = std::function<void(Frame&)>;

class Demuxer {
 public:
  /**
   * @brief Demuxer constructor
   *
   * @param inputSource - url of input source
   * @param inputFormat - format you want to force demuxer to use
   * @return FF_CPP_API
   */
  FF_CPP_API explicit Demuxer(const std::string& inputSource,
                              const std::string& inputFormat = "");
  FF_CPP_API ~Demuxer();

  /**
   * @brief Getter for input source
   */
  FF_CPP_API const std::string& inputSource() const;

  /**
   * @brief Prepare input to use
   * @exception OptionsNotAccepted - not all parameters passed in constructor
   * are accepted
   * @exception BadInput - open input failed
   * @exception NoStream - if find stream info failed
   * @exception TimeoutElapsed - if timeout elapsed while find stream info
   */
  FF_CPP_API void prepare(const ParametersContainer& params = {},
                          unsigned int timeout = 15);

  /**
   * @brief Return sources metadata
   *
   * @return MetadataContainer
   * in case of not prepared input return MetadataContainer{}
   */
  FF_CPP_API MetadataContainer metadata() const;

  /**
   * @brief Dump demuxer using av_dump_format()
   * @note if demuxer not prepared no info will be dumped
   */
  FF_CPP_API void dump() const;

  /**
   * @brief Vector of streams
   *
   * @return const std::vector<FFStream>
   */
  FF_CPP_API const std::vector<Stream>& streams() const;

  /**
   * @brief Find best video stream
   * @exception FFCppException - if demuxer not prepared
   * @exception NoStream - if find best video stream failed
   * @return FFStream
   */
  FF_CPP_API Stream& bestVideoStream() const;

  /**
   * @brief Create a Decoder object
   *
   * @param streamIndex you want to decode
   * @param requiredCodec user specified codec
   * @exception NoStream - if streamIndex out of range
   * @return FFDecoder&
   */
  FF_CPP_API Decoder& createDecoder(size_t streamIndex,
                                    AVCodecID requiredCodec = AV_CODEC_ID_NONE);

  /**
   * @brief All created decoders, key is stream index
   *
   * @return const& std::map<size_t, Decoder>
   */
  FF_CPP_API const std::map<size_t, Decoder>& decoders() const;

  /**
   * @brief Start demuxing/decoding routine, this is blocking function
   *
   * @param fc frame callback
   * @param pc packet callback
   * @note AVFrame/AVPacket received in callbacks are valid only during callback
   * call
   * @exception FFCppException if demuxer not prepared
   * @exception ProcessingError if error occured while demuxind\decoding routine
   * @exception EndOfFile if end of file reached while read frame from input
   * source
   * @exception TimeoutElapsed if timeout elapsed while read frames
   */
  FF_CPP_API void start(frame_callback fc = [](Frame&) {},
                        packet_callback pc = [](Packet&) { return true; });

  /**
   * @brief Stop demuxing/decoding routine
   */
  FF_CPP_API void stop();

  FF_CPP_API friend std::ostream& operator<<(std::ostream& ost,
                                             const Demuxer& dmxr);

 private:
  Demuxer(const Demuxer&) = delete;
  Demuxer(const Demuxer&&) = delete;
  Demuxer& operator=(const Demuxer&) = delete;
  Demuxer&& operator=(const Demuxer&&) = delete;

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace ff_cpp