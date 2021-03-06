#include <ff_cpp/ff_info.h>

#include <iostream>
#include <string>

namespace ff_cpp {

static void dumpProtocols(int protocolType) {
  void* opaque = nullptr;
  auto protocol = avio_enum_protocols(&opaque, protocolType);
  while (protocol) {
    std::cout << protocol << std::endl;
    protocol = avio_enum_protocols(&opaque, protocolType);
  }
}

void Info::dumpOutputProtocols() {
  std::cout << "==Output Protocols==" << std::endl;
  dumpProtocols(1);
}

void Info::dumpInputProtocols() {
  std::cout << "==Input Protocols==" << std::endl;
  dumpProtocols(0);
}

void Info::dumpOutputFormats() {
  void* opaque = nullptr;
  std::cout << "==Output Formats==" << std::endl;
  auto muxer = av_muxer_iterate(&opaque);
  while (muxer) {
    std::cout << muxer->name << " - " << muxer->long_name << std::endl;
    muxer = av_muxer_iterate(&opaque);
  }
}

void Info::dumpInputFormats() {
  void* opaque = nullptr;
  std::cout << "==Input Formats==" << std::endl;
  auto demuxer = av_demuxer_iterate(&opaque);
  while (demuxer) {
    std::cout << demuxer->name << " - " << demuxer->long_name << std::endl;
    demuxer = av_demuxer_iterate(&opaque);
  }
}

void Info::dumpInputVideoDevices() {
  AVInputFormat* inputVideoDevice{};
  inputVideoDevice = av_input_video_device_next(inputVideoDevice);
  if (inputVideoDevice) {
    std::cout << "==Input video devices==" << std::endl;
  }
  while (inputVideoDevice) {
    std::cout << inputVideoDevice->name << " - " << inputVideoDevice->long_name
              << std::endl;
    inputVideoDevice = av_input_video_device_next(inputVideoDevice);
  }
}

void Info::dumpOutputVideoDevices() {
  AVOutputFormat* outputVideoDevice{};
  outputVideoDevice = av_output_video_device_next(outputVideoDevice);
  if (outputVideoDevice) {
    std::cout << "==Input video devices==" << std::endl;
  }
  while (outputVideoDevice) {
    std::cout << outputVideoDevice->name << " - "
              << outputVideoDevice->long_name << std::endl;
    outputVideoDevice = av_output_video_device_next(outputVideoDevice);
  }
}

void Info::dumpCodecs() {
  void* opaque = nullptr;
  std::cout << "==Codecs==" << std::endl;
  auto codec = av_codec_iterate(&opaque);
  while (codec) {
    std::cout << codec->name << " - " << codec->long_name << std::endl;
    codec = av_codec_iterate(&opaque);
  }
}

void Info::dumpFilters() {
  void* opaque = nullptr;
  std::cout << "==Filters==" << std::endl;
  auto filter = av_filter_iterate(&opaque);
  while (filter) {
    std::cout << filter->name << " - " << filter->description << std::endl;
    filter = av_filter_iterate(&opaque);
  }
}

void Info::dumpConfiguration() {
  std::cout << avcodec_configuration() << std::endl;
}

}  // namespace ff_cpp