#include <ff_cpp/ff_scaler.h>

namespace ff_cpp {

using UniqSwsContext = std::unique_ptr<SwsContext, decltype(sws_freeContext)*>;

struct Scaler::Impl {
  int srcWidth{};
  int srcHeight{};
  AVPixelFormat srcFormat{};
  int dstWidth{};
  int dstHeight{};
  AVPixelFormat dstFormat{};

  UniqSwsContext swsContext{nullptr, sws_freeContext};
};

Scaler::Scaler(int width, int height, AVPixelFormat srcFormat,
               AVPixelFormat dstFormat) {
  impl_.reset(new Impl);
  impl_->srcWidth = width;
  impl_->srcHeight = height;
  impl_->srcFormat = srcFormat;
  impl_->dstWidth = width;
  impl_->dstHeight = height;
  impl_->dstFormat = dstFormat;

  impl_->swsContext.reset(
      sws_getContext(impl_->srcWidth, impl_->srcHeight, impl_->srcFormat,
                     impl_->dstWidth, impl_->dstHeight, impl_->dstFormat,
                     SWS_BILINEAR, nullptr, nullptr, nullptr));
  if (!impl_->swsContext) {
    throw std::runtime_error{"Unable to create sws context"};
  }
}

Scaler::~Scaler() = default;

ff_cpp::Frame Scaler::scale(ff_cpp::Frame& srcFrame, int dstAlignment) {
  if (srcFrame.width() != impl_->srcWidth ||
      srcFrame.height() != impl_->srcHeight ||
      srcFrame.format() != impl_->srcFormat) {
    throw std::runtime_error{"Unexpected src frame parameters"};
  }

  ff_cpp::Frame dstFrame{impl_->dstWidth, impl_->dstHeight, impl_->dstFormat,
                         dstAlignment};

  auto outputSliceHeight =
      sws_scale(impl_->swsContext.get(), srcFrame.data(), srcFrame.linesize(),
                0, srcFrame.height(), dstFrame.data(), dstFrame.linesize());

  if (outputSliceHeight != srcFrame.height()) {
    throw std::runtime_error{
        "Output slice height not the same as input frame height"};
  }

  return dstFrame;
}

ff_cpp::Frame& Scaler::scale(ff_cpp::Frame& srcFrame, ff_cpp::Frame& dstFrame) {
  if (srcFrame.width() != impl_->srcWidth ||
      srcFrame.height() != impl_->srcHeight ||
      srcFrame.format() != impl_->srcFormat) {
    throw std::runtime_error{"Unexpected src frame parameters"};
  }

  if (dstFrame.width() != impl_->dstWidth ||
      dstFrame.height() != impl_->dstHeight ||
      dstFrame.format() != impl_->dstFormat) {
    throw std::runtime_error{"Unexpected dst frame parameters"};
  }

  auto outputSliceHeight =
      sws_scale(impl_->swsContext.get(), srcFrame.data(), srcFrame.linesize(),
                0, srcFrame.height(), dstFrame.data(), dstFrame.linesize());

  if (outputSliceHeight != srcFrame.height()) {
    throw std::runtime_error{
        "Output slice height not the same as input frame height"};
  }

  return dstFrame;
}

}  // namespace ff_cpp