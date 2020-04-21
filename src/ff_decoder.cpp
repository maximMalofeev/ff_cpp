#include <ff_cpp/ff_decoder.h>
#include <ff_cpp/ff_exception.h>

namespace ff_cpp {

static void avCodecDeleter(AVCodecContext* ctxt) {
  if (ctxt) {
    avcodec_free_context(&ctxt);
  }
};
using UniqCodecContext =
    std::unique_ptr<AVCodecContext, decltype(avCodecDeleter)*>;

struct Decoder::Impl {
  UniqCodecContext decoderContext{nullptr, avCodecDeleter};
};

Decoder::Decoder(AVCodecID codecId, AVCodecParameters* codecpar,
                 const ParametersContainer& userParams) {
  impl_ = std::make_unique<Impl>();
  auto decoder = avcodec_find_decoder(codecId);
  if (!decoder) {
    throw NoDecoder(std::string{"Decoder for codec "} +
                        avcodec_get_name(codecId) + " not found",
                    codecId);
  }

  auto decoderContext = avcodec_alloc_context3(decoder);
  if (!decoderContext) {
    throw FFCppException("Decoder context not allocated");
  }

  impl_->decoderContext.reset(decoderContext);

  if (codecpar) {
    if (auto err = avcodec_parameters_to_context(impl_->decoderContext.get(),
                                                 codecpar);
        err < EXIT_SUCCESS) {
      throw FFCppException(
          std::string{"Codec params not copied to context, error: "} +
          av_err2str(err));
    }
  }

  AVDictionary* optionsDict{};
  for (const auto& param : userParams) {
    av_dict_set(&optionsDict, param.first.c_str(), param.second.c_str(), 0);
  }

  if (auto err =
          avcodec_open2(impl_->decoderContext.get(), decoder, &optionsDict);
      err != EXIT_SUCCESS) {
    throw FFCppException(std::string{"Codec open error, error: "} +
                         av_err2str(err));
  }

  if (optionsDict != nullptr) {
    throw OptionsNotAccepted("Not all options accepted", userParams);
  }
}

Decoder::Decoder(Decoder&& other) { impl_ = std::move(other.impl_); }

Decoder::~Decoder() {}

FF_CPP_API AVMediaType Decoder::mediaType() const {
  return impl_->decoderContext->codec_type;
}

FF_CPP_API AVCodecID Decoder::codec() const {
  return impl_->decoderContext->codec_id;
}

FF_CPP_API int Decoder::width() const { return impl_->decoderContext->width; }

FF_CPP_API int Decoder::height() const { return impl_->decoderContext->height; }

FF_CPP_API int Decoder::format() const {
  return impl_->decoderContext->pix_fmt;
}

int Decoder::sendPacket(Packet& pkt) const {
  return avcodec_send_packet(impl_->decoderContext.get(), pkt);
}

int Decoder::receiveFrame(Frame& frame) {
  return avcodec_receive_frame(impl_->decoderContext.get(), frame);
}

std::ostream& operator<<(std::ostream& ost, const Decoder& dcdr) {
  ost << "Decoder:\n";
  ost << "\tCodec: " << dcdr.impl_->decoderContext->codec->name << "("
      << dcdr.impl_->decoderContext->codec->long_name << ")\n";
  ost << "\tType: "
      << av_get_media_type_string(dcdr.impl_->decoderContext->codec_type);

  if (dcdr.impl_->decoderContext->codec_type == AVMEDIA_TYPE_VIDEO) {
    ost << "\n\tPixel format: "
        << av_get_pix_fmt_name(
               static_cast<AVPixelFormat>(dcdr.impl_->decoderContext->pix_fmt))
        << "\n";
    ost << "\tResolution: " << dcdr.impl_->decoderContext->width << "x"
        << dcdr.impl_->decoderContext->height;
  }
  return ost;
}

}  // namespace ff_cpp