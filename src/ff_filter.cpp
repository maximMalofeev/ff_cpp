#include "ff_cpp/ff_filter.h"

#include <ff_cpp/ff_exception.h>

#include <sstream>

namespace ff_cpp {

static void avFilterInOutDeleter(AVFilterInOut* inOut) {
  if (inOut) {
    avfilter_inout_free(&inOut);
  }
}
using UniqInOut =
    std::unique_ptr<AVFilterInOut, decltype(avFilterInOutDeleter)*>;

static void avFilterGrafDeleter(AVFilterGraph* graph) {
  if (graph) {
    avfilter_graph_free(&graph);
  }
}
using UniqGraph =
    std::unique_ptr<AVFilterGraph, decltype(avFilterGrafDeleter)*>;

struct Filter::Impl {
  std::string filterDescription;
  std::vector<int> allowedFormats{};
  AVFilterContext* bufferSinkCtx{};
  AVFilterContext* bufferSrcCtx{};
  UniqGraph filterGraph{nullptr, avFilterGrafDeleter};
};

Filter::Filter(const std::string& filterDescr, int width, int height,
               int format, const std::vector<int>& allowedFormats,
               bool autoConvert) {
  impl_ = std::make_unique<Impl>();
  impl_->filterDescription = filterDescr;
  impl_->allowedFormats = allowedFormats;

  const AVFilter* buffersrc = avfilter_get_by_name("buffer");
  const AVFilter* buffersink = avfilter_get_by_name("buffersink");
  UniqInOut outputs{avfilter_inout_alloc(), avFilterInOutDeleter};
  UniqInOut inputs{avfilter_inout_alloc(), avFilterInOutDeleter};
  impl_->filterGraph.reset(avfilter_graph_alloc());
  avfilter_graph_set_auto_convert(
      impl_->filterGraph.get(),
      autoConvert ? AVFILTER_AUTO_CONVERT_ALL : AVFILTER_AUTO_CONVERT_NONE);

  if (!outputs || !inputs || !impl_->filterGraph) {
    throw FFCppException(av_make_error_string(AVERROR(ENOMEM)));
  }

  // TODO: add ability customise this params
  AVRational timebase = {1, 25}, fps = {25, 1}, aspectRatio = {0, 1};

  std::ostringstream bufFilterDescr;
  bufFilterDescr << "video_size=" << width << "x" << height
                 << ":pix_fmt=" << format << ":time_base=" << timebase.num
                 << "/" << timebase.den << ":frame_rate=" << fps.num << "/"
                 << fps.den << ":pixel_aspect=" << aspectRatio.num << "/"
                 << aspectRatio.den;

  auto ret = avfilter_graph_create_filter(&impl_->bufferSrcCtx, buffersrc, "in",
                                          bufFilterDescr.str().c_str(), nullptr,
                                          impl_->filterGraph.get());
  if (ret < EXIT_SUCCESS) {
    throw FilterError("Unable to create buffer source, reason: " +
                      av_make_error_string(ret));
  }

  ret =
      avfilter_graph_create_filter(&impl_->bufferSinkCtx, buffersink, "out",
                                   nullptr, nullptr, impl_->filterGraph.get());
  if (ret < 0) {
    throw FilterError("Unable to create buffer sink, reason: " +
                      av_make_error_string(ret));
  }

  if (!impl_->allowedFormats.empty()) {
    impl_->allowedFormats.push_back(AV_PIX_FMT_NONE);
    ret = av_opt_set_int_list(impl_->bufferSinkCtx, "pix_fmts",
                              impl_->allowedFormats.data(), AV_PIX_FMT_NONE,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
      throw FilterError("Unable to set output pixel formats, reason: " +
                        av_make_error_string(ret));
    }
  }

  outputs->name = av_strdup("in");
  outputs->filter_ctx = impl_->bufferSrcCtx;
  outputs->pad_idx = 0;
  outputs->next = nullptr;

  inputs->name = av_strdup("out");
  inputs->filter_ctx = impl_->bufferSinkCtx;
  inputs->pad_idx = 0;
  inputs->next = nullptr;

  auto inputsPtr = inputs.release();
  auto outputsPtr = outputs.release();
  if ((ret = avfilter_graph_parse_ptr(impl_->filterGraph.get(),
                                      impl_->filterDescription.c_str(),
                                      &inputsPtr, &outputsPtr, nullptr)) < 0) {
    throw FilterError("Unable to parse graph, reason: " +
                      av_make_error_string(ret));
  }
  if ((ret = avfilter_graph_config(impl_->filterGraph.get(), nullptr)) < 0) {
    throw FilterError("Unable to config graph, reason: " +
                      av_make_error_string(ret));
  };
}

Filter::Filter(Filter&& other) {
  if (this == &other) {
    return;
  }
  impl_ = std::move(other.impl_);
}

Filter::~Filter() = default;

const std::string& Filter::filterDescription() const {
  return impl_->filterDescription;
}

Filter& Filter::operator=(Filter&& other) {
  if (this == &other) {
    return *this;
  }
  impl_ = std::move(other.impl_);
  return *this;
}

Frame Filter::filter(Frame& frm, bool keepRef) {
  int flags = AV_BUFFERSRC_FLAG_PUSH;
  if (keepRef) {
    flags |= AV_BUFFERSRC_FLAG_KEEP_REF;
  }
  int ret = av_buffersrc_add_frame_flags(impl_->bufferSrcCtx, frm, flags);
  if (ret < EXIT_SUCCESS) {
    throw ProcessingError("Unable to add frame buffer, reason: " +
                          ff_cpp::av_make_error_string(ret));
  }

  Frame outFrm;
  ret = av_buffersink_get_frame_flags(impl_->bufferSinkCtx, outFrm, 0);
  if (ret < EXIT_SUCCESS) {
    throw ProcessingError("Unable to get frame from sink, reason: " +
                          ff_cpp::av_make_error_string(ret));
  }

  return outFrm;
}

}  // namespace ff_cpp