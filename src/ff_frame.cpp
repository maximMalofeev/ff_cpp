#include <ff_cpp/ff_exception.h>
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
  void getBuffer(int width, int height, int format, int align) {
    frame->width = width;
    frame->height = height;
    frame->format = format;

    auto ret = av_frame_get_buffer(frame.get(), align);
    if (ret < EXIT_SUCCESS) {
      throw ff_cpp::FFCppException("Unable to alloc buffer, reason: " +
                                   av_make_error_string(ret));
    }
  }
};

Frame::Frame() { impl_ = std::make_unique<Impl>(); }

Frame::Frame(int width, int height, int format, int align) {
  impl_ = std::make_unique<Impl>();
  impl_->getBuffer(width, height, format, align);
}

Frame::Frame(const uint8_t* ptr, int width, int height, int format, int align) {
  impl_ = std::make_unique<Impl>();
  impl_->getBuffer(width, height, format, align);
  
  auto bufSize = av_image_get_buffer_size(static_cast<AVPixelFormat>(format),
                                          width, height, align);
  std::copy(ptr, ptr + bufSize, impl_->frame->data[0]);
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