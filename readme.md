# ff_cpp

Simple wrapper around ffmpeg.
It made to simplyfy process of demuxing and decoding inputs.
This library gives simple access to received AVPacket and decoded AVFrame

# Prerequires

CMake 3.10
Conan pacage manager
C++17

# Example of usage

```C++
ff_cpp::Demuxer demuxer("rtsp://some_ip:5555/Some/Stream/3");
try {
  demuxer.prepare({{"fflags", "autobsf+discardcorrupt+genpts+ignidx+igndts"},
                   {"rtsp_transport", "tcp"},
                   {"allowed_media_types", "video"}},
                  5); // or just demuxer.prepare() if no options required
  std::cout << demuxer << std::endl;
  auto& vStream = demuxer.bestVideoStream();

  demuxer.createDecoder(vStream.index());
  demuxer.start(
      [&](const AVFrame* frm) {
        // Work with AVFrame
      },
      [&demuxer](const AVPacket* pkt) {
        // Work with AVPacket, if you want to decode it return true, otherwise false
        if (pkt->stream_index == demuxer.bestVideoStream().index()) {
          return true;
        }
        return false;
      });
} catch (const ff_cpp::FFCppException& e) {
  std::cerr << "FF_CPP exception: " << e.what() << std::endl;
  return 1;
}
```