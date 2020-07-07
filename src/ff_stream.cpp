#include <ff_cpp/ff_stream.h>

#include <ostream>

namespace ff_cpp {

struct Stream::Impl {
  AVStream* stream{};
};

Stream::Stream(AVStream* stream) {
  impl_ = std::make_unique<Impl>();
  impl_->stream = stream;
}

Stream::Stream(Stream&& other) { impl_ = std::move(other.impl_); }

Stream::~Stream() {}

size_t Stream::index() const {
  return impl_->stream ? impl_->stream->index : -1;
}

AVMediaType Stream::mediaType() const {
  return impl_->stream ? impl_->stream->codecpar->codec_type
                       : AVMEDIA_TYPE_UNKNOWN;
}

AVCodecID Stream::codec() const {
  return impl_->stream ? impl_->stream->codecpar->codec_id : AV_CODEC_ID_NONE;
}

int Stream::width() const { return impl_->stream->codecpar->width; }

int Stream::height() const { return impl_->stream->codecpar->height; }

int Stream::format() const { return impl_->stream->codecpar->format; }

AVRational Stream::averageFPS() const {
  if (impl_->stream->avg_frame_rate.num == 0 &&
      impl_->stream->avg_frame_rate.den == 0) {
    return AVRational{0, 1};
  }
  return impl_->stream->avg_frame_rate;
}

AVRational Stream::timeBase() const { return impl_->stream->time_base; }

AVRational Stream::pixelAspectRatio() const {
  return impl_->stream->sample_aspect_ratio;
}

std::ostream& operator<<(std::ostream& ost, const Stream& s) {
  ost << "Stream:\n";
  if (s.impl_->stream) {
    ost << "\tIndex: " << s.impl_->stream->index << "\n";
    ost << "\tCodec: " << avcodec_get_name(s.impl_->stream->codecpar->codec_id)
        << "\n";
    ost << "\tType: "
        << av_get_media_type_string(s.impl_->stream->codecpar->codec_type)
        << "\n";
    ost << "\tResolution: " << s.impl_->stream->codecpar->width << "x"
        << s.impl_->stream->codecpar->height << "\n";
    ost << "\tAverage FPS: "
        << s.impl_->stream->avg_frame_rate.num /
               s.impl_->stream->avg_frame_rate.den
        << "\n";
    ost << "\tTime base: " << s.impl_->stream->time_base.num << "/"
        << s.impl_->stream->time_base.den;
  } else {
    ost << "\tEmpty stream";
  }

  return ost;
}

}  // namespace ff_cpp