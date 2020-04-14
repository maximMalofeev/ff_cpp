#include <ff_cpp/ff_frame.h>

namespace ff_cpp {

static void avFrameDeleter(AVFrame* frame) {
  av_frame_unref(frame);
  av_freep(&frame->data[0]);
  av_frame_free(&frame);
}
using UniqFrame = std::unique_ptr<AVFrame, decltype(avFrameDeleter)*>;

struct Frame::Impl {
  UniqFrame frame{av_frame_alloc(), avFrameDeleter};
};

Frame::Frame() { impl_ = std::make_unique<Impl>(); }

Frame::Frame(int width, int height, int format, int align) {
  impl_ = std::make_unique<Impl>();
  av_image_alloc(impl_->frame->data, impl_->frame->linesize, width, height,
                 static_cast<AVPixelFormat>(format), align);
  impl_->frame->width = width;
  impl_->frame->height = height;
  impl_->frame->format = format;
}

Frame::Frame(uint8_t* ptr, int width, int height, int format, int align) {
  impl_ = std::make_unique<Impl>();
  av_image_fill_arrays(impl_->frame->data, impl_->frame->linesize, ptr,
                       static_cast<AVPixelFormat>(format), width, height,
                       align);
  impl_->frame->width = width;
  impl_->frame->height = height;
  impl_->frame->format = format;
}

Frame::Frame(Frame&& other) { impl_ = std::move(other.impl_); }

Frame::~Frame() = default;

int Frame::width() const { return impl_->frame->width; }

int Frame::height() const { return impl_->frame->height; }

int Frame::format() const { return impl_->frame->format; }

int64_t Frame::pts() const { return impl_->frame->pts; }

int64_t Frame::dts() const { return impl_->frame->pkt_dts; }

int Frame::numDataPointers() const { return AV_NUM_DATA_POINTERS; }

uint8_t** Frame::data() const { return impl_->frame->data; }

int* Frame::linesize() const { return impl_->frame->linesize; }

Frame::operator AVFrame*() { return impl_->frame.get(); }

std::ostream& operator<<(std::ostream& ost, const Frame& frame) {
  ost << "Frame:\n";
  ost << "\tWidth: " << frame.width() << "\n";
  ost << "\tHeight: " << frame.height() << "\n";
  ost << "\tFormat: " << frame.format() << "\n";
  ost << "\tPts: " << frame.pts();
  return ost;
}

}  // namespace ff_cpp