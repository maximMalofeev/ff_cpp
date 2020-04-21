#include <ff_cpp/ff_exception.h>
#include <ff_cpp/ff_frame.h>

namespace ff_cpp {

static void avFrameDeleter(AVFrame* frame) {
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
  av_image_fill_arrays(impl_->frame->data, impl_->frame->linesize, ptr,
                       static_cast<AVPixelFormat>(format), width, height,
                       align);
  impl_->frame->extended_data = impl_->frame->data;           
  impl_->frame->width = width;
  impl_->frame->height = height;
  impl_->frame->format = format;
  impl_->frame->key_frame = 1;

  //No pallet fot y8 images
  if(format == AV_PIX_FMT_GRAY8){
    impl_->frame->data[1] = nullptr;
  }
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