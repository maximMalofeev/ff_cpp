![C/C++ CI](https://github.com/maximMalofeev/ff_cpp/workflows/C/C++%20CI/badge.svg?branch=master)

# ff_cpp

Simple wrapper around ffmpeg.  
I made this library to simplify the process of demuxing, decoding and filtering inputs. This library gives simple access to received AVPacket and decoded AVFrame.

# Prerequires

CMake 3.10  
Conan package manager  
C++17  

# Example of usage

```C++
ff_cpp::Demuxer demuxer{"rtsp://some_ip:5555/Some/Stream/3"};
try {
  demuxer.prepare({{"fflags", "autobsf+discardcorrupt+genpts+ignidx+igndts"},
                   {"rtsp_transport", "tcp"},
                   {"allowed_media_types", "video"}},
                  5);
  /*
  or just demuxer.prepare() if no options required

  or if you want to open web camera
  ff_cpp::Demuxer demuxer("video=USB2.0 UVC HQ WebCam", "dshow");
  try {
    demuxer.prepare({{"video_size","hd720"}});
    ...
  }catch (const ff_cpp::FFCppException& e) {
    ...
  }
  ...
  */
  std::cout << demuxer << std::endl;
  auto& vStream = demuxer.bestVideoStream();

  ff_cpp::Filter filter{"boxblur=10", vStream.width(), vStream.height(),
                        vStream.format(), {vStream.format()}};

  ff_cpp::Scaler scaler{vStream.width(), vStream.height(), vStream.format(), AV_PIX_FMT_GRAY8};

  demuxer.createDecoder(vStream.index());
  demuxer.start(
      [&](ff_cpp::Frame& frm) {
        // Work with Frame...

        // Filter original frame
        auto filteredFrm = filter.filter(frm);
        // Work withfiltered frame...

        // Scale frame
        auto scaledFrame = scaler.scale(frm, 32);
      },
      [&demuxer](ff_cpp::Packet& pkt) {
        // Work with Packet, if you want to decode it return true, otherwise false
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
