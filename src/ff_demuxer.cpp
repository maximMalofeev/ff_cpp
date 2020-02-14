#include <ff_cpp/ff_demuxer.h>
#include <ff_cpp/ff_exception.h>
#include <ff_cpp/ff_stream.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>

namespace ff_cpp {

static void avFormatDeleater(AVFormatContext* ctxt) {
  avformat_close_input(&ctxt);
};
using UniqFormatContext =
    std::unique_ptr<AVFormatContext, decltype(avFormatDeleater)*>;

static void avFrameDeleter(AVFrame* frame) { av_frame_free(&frame); }
using UniqFrame = std::unique_ptr<AVFrame, decltype(avFrameDeleter)*>;

struct Demuxer::Impl {
  std::string input;
  UniqFormatContext demuxerContext{nullptr, avFormatDeleater};
  std::vector<Stream> streams;
  std::map<int, Decoder> decoders;

  bool doWork{};

  volatile bool timeoutElapsed{};
  std::chrono::seconds timeout;
  std::chrono::steady_clock::time_point timePoint{};

  /**
   * @brief set start time point of new ffmpeg request
   */
  void updateRequestTime() { timePoint = std::chrono::steady_clock::now(); }

  static int interrupt_callback(void* opaque) {
    auto demuxer = static_cast<Demuxer*>(opaque);
    if (demuxer) {
      auto now = std::chrono::steady_clock::now();
      auto timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(
          now - demuxer->impl_->timePoint);
      if (demuxer->impl_->timeout <= timeElapsed) {
        demuxer->impl_->timeoutElapsed = true;
        return 1;
      } else {
        return 0;
      }
    }
    return 1;
  }
};

Demuxer::Demuxer(const std::string& inputSource) {
  impl_ = std::make_unique<Impl>();
  impl_->input = inputSource;
}

Demuxer::~Demuxer() {}

const std::string& Demuxer::inputSource() const { return impl_->input; }

void Demuxer::prepare(ParametersContainer params, unsigned int timeout) {
  AVFormatContext* fmtCntxt = avformat_alloc_context();
  fmtCntxt->interrupt_callback.callback = Impl::interrupt_callback;
  fmtCntxt->interrupt_callback.opaque = this;

  AVDictionary* optionsDict{};
  for (const auto& param : params) {
    av_dict_set(&optionsDict, param.first.c_str(), param.second.c_str(), 0);
  }

  impl_->timeout = std::chrono::seconds{timeout};
  impl_->updateRequestTime();

  if (auto err = avformat_open_input(&fmtCntxt, impl_->input.c_str(), nullptr,
                                     &optionsDict);
      err == EXIT_SUCCESS) {
    impl_->demuxerContext.reset(fmtCntxt);

    if (err = avformat_find_stream_info(impl_->demuxerContext.get(), nullptr);
        err < EXIT_SUCCESS) {
      if (impl_->timeoutElapsed) {
        throw TimeoutElapsed("Timeout elapsed while find stream info");
      }
      throw NoStream(av_err2str(err));
    }

    for (unsigned int i = 0; i < impl_->demuxerContext->nb_streams; i++) {
      impl_->demuxerContext->streams[i]->discard = AVDISCARD_DEFAULT;
      impl_->streams.emplace_back(Stream{impl_->demuxerContext->streams[i]});
    }

    if (optionsDict != nullptr) {
      AVDictionaryEntry* opt = nullptr;
      ParametersContainer params;
      while ((opt = av_dict_get(optionsDict, "", opt, AV_DICT_IGNORE_SUFFIX))) {
        params[opt->key] = opt->value;
      }
      throw OptionsNotAccepted("Not all options accepted", params);
    }
  } else {
    if (impl_->timeoutElapsed) {
      throw TimeoutElapsed("Timeout elapsed while open input");
    }
    throw BadInput(av_err2str(err), impl_->input);
  }
}

MetadataContainer Demuxer::metadata() const {
  if (!impl_->demuxerContext) {
    return {};
  }
  AVDictionaryEntry* tag = nullptr;
  MetadataContainer metadata;
  while ((tag = av_dict_get(impl_->demuxerContext->metadata, "", tag,
                            AV_DICT_IGNORE_SUFFIX))) {
    metadata[tag->key] = tag->value;
  }

  return metadata;
}

void Demuxer::dump() const {
  if (impl_->demuxerContext) {
    av_dump_format(impl_->demuxerContext.get(), 0, impl_->input.c_str(), 0);
  }
}

const std::vector<Stream>& Demuxer::streams() const { return impl_->streams; }

Stream& Demuxer::bestVideoStream() const {
  if (!impl_->demuxerContext) {
    throw FFCppException("Demuxer not prepared");
  }
  auto streamIndex = av_find_best_stream(
      impl_->demuxerContext.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (streamIndex < EXIT_SUCCESS) {
    throw NoStream(av_err2str(streamIndex));
  }

  return impl_->streams[streamIndex];
}

Decoder& Demuxer::createDecoder(int streamIndex, AVCodecID requiredCodec) {
  if (streamIndex >= impl_->streams.size()) {
    throw NoStream("There is no stream with such index");
  }

  auto& stream = impl_->streams[streamIndex];

  AVCodecID codec =
      requiredCodec == AV_CODEC_ID_NONE ? stream.codec() : requiredCodec;
  impl_->decoders.emplace(
      stream.index(),
      Decoder{codec, impl_->demuxerContext->streams[stream.index()]->codecpar});
  return impl_->decoders.at(stream.index());
}

const std::map<int, Decoder>& Demuxer::decoders() const {
  return impl_->decoders;
}

void Demuxer::start(frame_callback fc, packet_callback pc) {
  if (!impl_->demuxerContext) {
    throw FFCppException("Demuxer not prepared");
  }

  AVPacket packet;
  UniqFrame frame{av_frame_alloc(), avFrameDeleter};
  int err = EXIT_SUCCESS;
  impl_->doWork = true;

  impl_->timeout = std::chrono::seconds{5};

  while (impl_->doWork) {
    impl_->updateRequestTime();
    if ((err = av_read_frame(impl_->demuxerContext.get(), &packet)) <
        EXIT_SUCCESS) {
      if (impl_->timeoutElapsed) {
        throw TimeoutElapsed("Timeout elapsed while read frame");
      }
      throw ProcessingError(std::string{"av_read_frame error: "} +
                            av_err2str(err));
    }

    if (pc(&packet) &&
        (impl_->decoders.find(packet.stream_index) != impl_->decoders.end())) {
      auto& decoder = impl_->decoders.at(packet.stream_index);
      impl_->updateRequestTime();
      if (err = decoder.sendPacket(&packet); err >= EXIT_SUCCESS) {
        err = EXIT_SUCCESS;
        while (err >= EXIT_SUCCESS) {
          err = decoder.receiveFrame(frame.get());
          if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
            break;
          } else if (err == AVERROR(EINVAL) || err < 0) {
            throw ProcessingError(av_err2str(err));
          }

          fc(frame.get());
        }
      } else {
        throw ProcessingError(av_err2str(err));
      }
    } else {
      av_packet_unref(&packet);
    }
  }
}

void Demuxer::stop() { impl_->doWork = false; }

std::ostream& operator<<(std::ostream& ost, const Demuxer& dmxr) {
  ost << "Demuxer:\n";
  if (!dmxr.impl_->demuxerContext) {
    ost << "\tNot prepared";
    return ost;
  }
  ost << "\tUrl: " << dmxr.impl_->demuxerContext->url << "\n";
  ost << "\tContainer: " << dmxr.impl_->demuxerContext->iformat->name << " ("
      << dmxr.impl_->demuxerContext->iformat->long_name << ")\n";
  auto metadata = dmxr.metadata();
  if (metadata.size()) {
    ost << "\tMetadata:\n";
    std::for_each(metadata.begin(), metadata.end(),
                  [&ost](MetadataContainer::value_type item) {
                    ost << "\t\t" << item.first << "=" << item.second << "\n";
                  });
  }

  if (dmxr.impl_->demuxerContext->nb_streams) {
    ost << "\tStreams:";
    int currentStream{};
    std::for_each(
        dmxr.impl_->demuxerContext->streams,
        dmxr.impl_->demuxerContext->streams +
            dmxr.impl_->demuxerContext->nb_streams,
        [&ost, &currentStream](AVStream* stream) {
          ost << "\n\t\tStream[" << currentStream++ << "]:\n";
          ost << "\t\t\t"
              << "Type: "
              << av_get_media_type_string(stream->codecpar->codec_type) << "\n";
          ost << "\t\t\t"
              << "Codec: " << avcodec_get_name(stream->codecpar->codec_id)
              << "\n";
          if (stream->codecpar &&
              stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ost << "\t\t\t"
                << "Pixel format: "
                << av_get_pix_fmt_name(
                       static_cast<AVPixelFormat>(stream->codecpar->format))
                << "\n";
            ost << "\t\t\t"
                << "Resolution: " << stream->codecpar->width << "x"
                << stream->codecpar->height << "\n";
            ost << "\t\t\t"
                << "Average fps: "
                << stream->avg_frame_rate.num / stream->avg_frame_rate.den;
          }
        });
  }

  return ost;
}

}  // namespace ff_cpp