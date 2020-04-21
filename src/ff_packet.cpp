#include <ff_cpp/ff_packet.h>

namespace ff_cpp {

static void avPacketDeleter(AVPacket* pkt) {
  if (pkt) {
    if (pkt->buf) {
      av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
  }
}
using UniqPacket = std::unique_ptr<AVPacket, decltype(avPacketDeleter)*>;

struct Packet::Impl {
  UniqPacket packet{av_packet_alloc(), avPacketDeleter};
};

Packet::Packet() {
  impl_ = std::make_unique<Impl>();
  av_init_packet(impl_->packet.get());
}

Packet::~Packet() = default;

int64_t Packet::pts() const { return impl_->packet->pts; }

int64_t Packet::dts() const { return impl_->packet->dts; }

int Packet::streamIndex() const { return impl_->packet->stream_index; }

Packet::operator AVPacket*() { return impl_->packet.get(); }

std::ostream& operator<<(std::ostream& ost, const Packet& pkt) {
  ost << "Packet:\n";
  ost << "\tPts: " << pkt.pts() << "\n";
  ost << "\tDts: " << pkt.dts() << "\n";
  ost << "\tStream index: " << pkt.streamIndex();
  return ost;
}

}  // namespace ff_cpp