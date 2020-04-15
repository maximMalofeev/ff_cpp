#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
                           // this in one cpp file
#include <ff_cpp/ff_demuxer.h>
#include <ff_cpp/ff_exception.h>
#include <ff_cpp/ff_filter.h>
#include <ff_cpp/ff_packet.h>
#include <ff_cpp/ff_frame.h>

#include <catch2/catch.hpp>
#include <fstream>
#include <iterator>
#include <algorithm>

const std::string url("file:small_bunny_1080p_60fps.mp4");
const std::string emptyFileUrl("file:empty_file.mp4");

TEST_CASE("Prepare demuxer", "[demuxer]") {
  SECTION("Empty url and empty parameters") {
    ff_cpp::Demuxer demuxer("");
    REQUIRE_THROWS_AS(demuxer.prepare(), ff_cpp::BadInput);
  }
  SECTION("Invalid url") {
    ff_cpp::Demuxer demuxer("invalid\\url");
    REQUIRE_THROWS_AS(demuxer.prepare(), ff_cpp::BadInput);
  }
  SECTION("Invalid parameters") {
    ff_cpp::Demuxer demuxer(url);
    REQUIRE_THROWS_AS(demuxer.prepare({{"invalid", "parameter"}}),
                      ff_cpp::OptionsNotAccepted);
  }
  SECTION("Empty file") {
    ff_cpp::Demuxer demuxer(emptyFileUrl);
    REQUIRE_THROWS_AS(demuxer.prepare(), ff_cpp::BadInput);
  }
  SECTION("Valid input") {
    ff_cpp::Demuxer demuxer(url);
    REQUIRE_NOTHROW(demuxer.prepare());
    std::cout << demuxer << std::endl;
  }
  SECTION("Timeout") {
    ff_cpp::Demuxer demuxer("rtsp://localhost:5555/Some/Stream/3");
    REQUIRE_THROWS_AS(demuxer.prepare({}, 5), ff_cpp::TimeoutElapsed);
  }
}

TEST_CASE("Operator <<", "[demuxer]") {
  SECTION("Not prepared demuxer") {
    ff_cpp::Demuxer demuxer(url);
    std::stringstream ss;
    REQUIRE_NOTHROW(ss << demuxer);
  }
  SECTION("Prepared demuxer") {
    ff_cpp::Demuxer demuxer(url);
    std::stringstream ss;
    REQUIRE_NOTHROW(ss << demuxer);
  }
  SECTION("Invalid demuxer") {
    ff_cpp::Demuxer demuxer("invalid\\url");
    REQUIRE_THROWS(demuxer.prepare());
    std::stringstream ss;
    REQUIRE_NOTHROW(ss << demuxer);
  }
}

TEST_CASE("Misc", "[demuxer]") {
  ff_cpp::Demuxer demuxer("");
  REQUIRE_NOTHROW(demuxer.dump());
  REQUIRE_NOTHROW(demuxer.metadata());
}

TEST_CASE("Demuxer streams", "[demuxer]") {
  SECTION("Not prepared demuxer has not any streams") {
    ff_cpp::Demuxer demuxer("");
    REQUIRE(demuxer.streams().empty());
    REQUIRE_THROWS_AS(demuxer.bestVideoStream(), ff_cpp::FFCppException);
  }
  SECTION("Prepared demuxer has have at least 1 stream") {
    ff_cpp::Demuxer demuxer(url);
    demuxer.prepare();
    REQUIRE(!demuxer.streams().empty());
    REQUIRE_NOTHROW(demuxer.bestVideoStream());
    auto& bestVStream = demuxer.bestVideoStream();
    std::cout << bestVStream << std::endl;
    REQUIRE((bestVStream.index() >= 0 &&
             bestVStream.index() < demuxer.streams().size()));
    REQUIRE(bestVStream.mediaType() == AVMEDIA_TYPE_VIDEO);
    REQUIRE(bestVStream.codec() == AV_CODEC_ID_H264);
    REQUIRE(bestVStream.width() == 1920);
    REQUIRE(bestVStream.height() == 1080);
    REQUIRE(bestVStream.format() ==
            static_cast<AVPixelFormat>(AV_PIX_FMT_YUV420P));
    REQUIRE(bestVStream.averageFPS().num / bestVStream.averageFPS().den == 60);
  }
}

TEST_CASE("Demuxer decoders", "[demuxer]") {
  SECTION("Decoder creation for not existing stream index") {
    ff_cpp::Demuxer demuxer(url);
    demuxer.prepare();
    REQUIRE_THROWS_AS(
        demuxer.createDecoder(static_cast<int>(demuxer.streams().size())),
        ff_cpp::NoStream);
  }
  SECTION("Decoder creation for best video stream") {
    ff_cpp::Demuxer demuxer(url);
    demuxer.prepare();
    REQUIRE_NOTHROW(demuxer.createDecoder(demuxer.bestVideoStream().index()));
    auto& decoder = demuxer.decoders().at(demuxer.bestVideoStream().index());
    std::cout << decoder << std::endl;
    REQUIRE(decoder.mediaType() == AVMEDIA_TYPE_VIDEO);
    REQUIRE(decoder.codec() == AV_CODEC_ID_H264);
    REQUIRE(decoder.width() == 1920);
    REQUIRE(decoder.height() == 1080);
    REQUIRE(decoder.format() == static_cast<AVPixelFormat>(AV_PIX_FMT_YUV420P));
  }
  SECTION("Create decoder twice for the same index") {
    ff_cpp::Demuxer demuxer(url);
    demuxer.prepare();
    REQUIRE_NOTHROW(demuxer.createDecoder(demuxer.bestVideoStream().index()));
    REQUIRE_NOTHROW(demuxer.createDecoder(demuxer.bestVideoStream().index()));
  }
}

TEST_CASE("Start/Stop demuxer", "[demuxer]") {
  SECTION("Start not prepared demuxer") {
    ff_cpp::Demuxer demuxer(url);
    REQUIRE_THROWS_AS(demuxer.start(), ff_cpp::FFCppException);
  }
  SECTION("Start and check packet and frame") {
    ff_cpp::Demuxer demuxer(url);
    demuxer.prepare();
    demuxer.createDecoder(demuxer.bestVideoStream().index());
    demuxer.start(
        [&demuxer](const ff_cpp::Frame& frm) {
          REQUIRE(frm.width() == 1920);
          REQUIRE(frm.height() == 1080);
          REQUIRE(frm.format() ==
                  static_cast<AVPixelFormat>(AV_PIX_FMT_YUV420P));
          demuxer.stop();
        },
        [&demuxer](const ff_cpp::Packet& pkt) {
          if (pkt.streamIndex() == demuxer.bestVideoStream().index()) {
            return true;
          }
          return false;
        });
  }
}

TEST_CASE("Packet tests", "[packet]") {
  SECTION("Contruction/Destruction") {
    { 
      ff_cpp::Packet pkt; 
      REQUIRE(pkt.pts() == AV_NOPTS_VALUE);
      REQUIRE(pkt.dts() == AV_NOPTS_VALUE);
      REQUIRE(pkt.streamIndex() == 0);
    }
  }
}

TEST_CASE("Frame tests", "[frame]") {
  SECTION("Default Contruction/Destruction") {
    { 
      ff_cpp::Frame frame;
      REQUIRE(frame.width() == 0);
      REQUIRE(frame.height() == 0);
      REQUIRE(frame.format() == -1);
      REQUIRE(frame.pts() == AV_NOPTS_VALUE);
      REQUIRE(frame.dts() == AV_NOPTS_VALUE);
      for(int i = 0; i < frame.numDataPointers(); i++){
        REQUIRE(frame.data()[i] == nullptr);
        REQUIRE(frame.linesize()[i] == 0);
      }
    }
  }
  SECTION("Parameterized Contruction/Destruction") {
    {
      constexpr int width = 1920;
      constexpr int height = 1080;
      constexpr int format = AV_PIX_FMT_RGB24;
      constexpr int bytesPerPixel = 3;
      ff_cpp::Frame frame{width, height, format};
      REQUIRE(frame.width() == width);
      REQUIRE(frame.height() == height);
      REQUIRE(frame.format() == format);
      REQUIRE(frame.pts() == AV_NOPTS_VALUE);
      REQUIRE(frame.dts() == AV_NOPTS_VALUE);

      REQUIRE(frame.data()[0] != nullptr);
      REQUIRE(frame.linesize()[0] == width * bytesPerPixel);
      for(int i = 1; i < frame.numDataPointers(); i++){
        REQUIRE(frame.data()[i] == nullptr);
        REQUIRE(frame.linesize()[i] == 0);
      }
    }
  }
  SECTION("Construct from allocated buffer"){
      constexpr int width = 1920;
      constexpr int height = 1080;
      constexpr int format = AV_PIX_FMT_RGB24;
      constexpr int bytesPerPixel = 3;
      constexpr uint8_t magicNum = 222;
      std::unique_ptr<uint8_t[]> buf{new uint8_t[width * height * bytesPerPixel]};
      buf[0] = magicNum;
      {
        ff_cpp::Frame frame{buf.get(), width, height, format};
        REQUIRE(frame.width() == width);
        REQUIRE(frame.height() == height);
        REQUIRE(frame.format() == format);
        REQUIRE(frame.pts() == AV_NOPTS_VALUE);
        REQUIRE(frame.dts() == AV_NOPTS_VALUE);
  
        REQUIRE(frame.data()[0] != nullptr);
        REQUIRE(frame.data()[0] == buf.get());
        REQUIRE(frame.linesize()[0] == width * bytesPerPixel);
        for(int i = 1; i < frame.numDataPointers(); i++){
          REQUIRE(frame.data()[i] == nullptr);
          REQUIRE(frame.linesize()[i] == 0);
        }
      }
      REQUIRE(buf[0] == magicNum);
  }
  SECTION("Check if alignment work"){
    constexpr int width = 1918;
    constexpr int height = 1080;
    constexpr int format = AV_PIX_FMT_GRAY8;
    ff_cpp::Frame frame{width, height, format, 4};
    REQUIRE(frame.width() == width);
    REQUIRE(frame.height() == height);
    REQUIRE(frame.format() == format);
    REQUIRE(frame.linesize()[0] == 1920);
  }
}

TEST_CASE("Filter tests", "[filter]") {
  const std::string filterDescr = "boxblur=10";
  SECTION("Filter creation") {
    ff_cpp::Filter f(filterDescr, 1920, 1080, AV_PIX_FMT_YUV420P);
    REQUIRE(filterDescr == f.filterDescription());
    auto f_moved = std::move(f);
    REQUIRE(filterDescr == f_moved.filterDescription());
  }
  SECTION("Boxblure filter") {
    constexpr int width = 1920;
    constexpr int height = 1080;
    constexpr int bytesPerPixel = 3;
    constexpr int format = AV_PIX_FMT_RGB24;
    constexpr int imgSize = width * height * bytesPerPixel;

    std::ifstream f("rgb24_1920_1080.data", std::ifstream::binary);
    REQUIRE(f);

    std::unique_ptr<char[]> img(new char[imgSize]);
    f.read(img.get(), imgSize);
    REQUIRE(f);
    f.close();

    ff_cpp::Frame imgFrm {reinterpret_cast<uint8_t*>(img.get()), width, height, format};

    ff_cpp::Filter filter(filterDescr, width, height, format, {format});

    auto filteredFrm = filter.filter(imgFrm);
    REQUIRE(imgFrm.width() == width);
    REQUIRE(imgFrm.height() == height);
    REQUIRE(imgFrm.format() == format);
    REQUIRE(filteredFrm.width() == width);
    REQUIRE(filteredFrm.height() == height);
    REQUIRE(filteredFrm.format() == format);
  }
  SECTION("Format filter"){
    constexpr int width = 1920;
    constexpr int height = 1080;
    constexpr int format = AV_PIX_FMT_RGB24;
    const std::string filterDescr = "format=pix_fmts=yuv420p";

    ff_cpp::Frame inFrm {width, height, format};

    ff_cpp::Filter filter(filterDescr, width, height, format);
    auto filteredFrm = filter.filter(inFrm);

    REQUIRE(inFrm.width() == width);
    REQUIRE(inFrm.height() == height);
    REQUIRE(inFrm.format() == format);
    REQUIRE(filteredFrm.width() == width);
    REQUIRE(filteredFrm.height() == height);
    REQUIRE(filteredFrm.format() == AV_PIX_FMT_YUV420P);
  }
  SECTION("Format filter and input frame with the same pix fmt"){
    constexpr int width = 1920;
    constexpr int height = 1080;
    constexpr int format = AV_PIX_FMT_RGB24;
    constexpr int imgSize = width * height * 3;
    const std::string filterDescr = "format=pix_fmts=rgb24";

    ff_cpp::Frame inFrm {width, height, format};

    ff_cpp::Filter filter(filterDescr, width, height, format);
    auto filteredFrm = filter.filter(inFrm);

    REQUIRE(inFrm.width() == width);
    REQUIRE(inFrm.height() == height);
    REQUIRE(inFrm.format() == format);
    REQUIRE(filteredFrm.width() == width);
    REQUIRE(filteredFrm.height() == height);
    REQUIRE(filteredFrm.format() == format);
    REQUIRE(std::equal(inFrm.data()[0], inFrm.data()[0] + imgSize, filteredFrm.data()[0]));
  }
  SECTION("More than one filter"){
    constexpr int width = 1920;
    constexpr int height = 1080;
    constexpr int bytesPerPixel = 3;
    constexpr int format = AV_PIX_FMT_RGB24;
    const std::string filterDescr = "boxblur=10,format=pix_fmts=yuv420p";

    ff_cpp::Frame inFrm {width, height, format};

    ff_cpp::Filter filter(filterDescr, width, height, format);
    auto filteredFrm = filter.filter(inFrm);

    REQUIRE(inFrm.width() == width);
    REQUIRE(inFrm.height() == height);
    REQUIRE(inFrm.format() == format);
    REQUIRE(filteredFrm.width() == width);
    REQUIRE(filteredFrm.height() == height);
    REQUIRE(filteredFrm.format() == AV_PIX_FMT_YUV420P);
  }
  SECTION("Curves filter"){
    constexpr int width = 1920;
    constexpr int height = 1080;
    constexpr int format = AV_PIX_FMT_GRAY8;
    const std::string filterDescr = "curves=all='0/0 0.45/0.45 0.5/0.3 0.75/0.75 1/1'";

    ff_cpp::Frame inFrm {width, height, format};

    ff_cpp::Filter filter(filterDescr, width, height, format, {format});
    auto filteredFrm = filter.filter(inFrm);

    REQUIRE(inFrm.width() == width);
    REQUIRE(inFrm.height() == height);
    REQUIRE(inFrm.format() == format);
    REQUIRE(filteredFrm.width() == width);
    REQUIRE(filteredFrm.height() == height);
    REQUIRE(filteredFrm.format() == format);
  }
  SECTION("Filter frame with aligned buffer"){
    constexpr int width = 1918;
    constexpr int height = 1080;
    constexpr int format = AV_PIX_FMT_GRAY8;
    constexpr int imgSize = width * height * 1;
    const std::string filterDescr = "curves=all='0/0 0.45/0.45 0.5/0.3 0.75/0.75 1/1'";

    ff_cpp::Frame inFrm {width, height, format, 4};

    ff_cpp::Filter filter(filterDescr, width, height, format, {format});
    auto filteredFrm = filter.filter(inFrm);

    REQUIRE(filteredFrm.width() == width);
    REQUIRE(filteredFrm.height() == height);
    REQUIRE(filteredFrm.format() == format);
    REQUIRE(filteredFrm.linesize()[0] == 1920);
  }
  SECTION("Test filter"){
    constexpr int width = 976;
    constexpr int height = 976;
    constexpr int format = AV_PIX_FMT_RGB24;
    constexpr int imgSize = width * height * 3;
    //const std::string filterDescr = "curves=all='0/0 0.45/0.45 0.5/0.3 0.75/0.75 1/1'";
    const std::string filterDescr = "format=pix_fmts=rgb24";

    ff_cpp::Filter filter(filterDescr, width, height, format, {format});

    // std::ifstream f{"976x976_rgb24.data", std::ios::binary};
    // REQUIRE(f);

    std::unique_ptr<char[]> img(new char[imgSize]);
    for(auto i = 0; i < imgSize; i++){
      img[i] = imgSize - i;
    }
    // f.read(img.get(), imgSize);
    // REQUIRE(f);
    // f.close();

    ff_cpp::Frame inFrm {reinterpret_cast<uint8_t*>(img.get()), width, height, format, 4};

    const std::string pngFileUrl("file:976x976.png");
    ff_cpp::Demuxer demuxer(pngFileUrl);
    demuxer.prepare({{"pixel_format", "rgb24"}});
    demuxer.createDecoder(demuxer.bestVideoStream().index());
    demuxer.start(
        [&](const ff_cpp::Frame& frm) {
          REQUIRE(frm.width() == width);
          REQUIRE(frm.height() == height);
          REQUIRE(frm.format() ==
                  static_cast<AVPixelFormat>(format));
          demuxer.stop();

          auto filteredFrm = filter.filter(const_cast<ff_cpp::Frame&>(frm));
          REQUIRE(std::equal(frm.data()[0], frm.data()[0] + imgSize, filteredFrm.data()[0]));

          ff_cpp::Frame ff{frm.data()[0], frm.width(), frm.height(), frm.format(), 1};
          auto ffFrm = filter.filter(const_cast<ff_cpp::Frame&>(ff));
          REQUIRE(std::equal(ffFrm.data()[0], ffFrm.data()[0] + imgSize, frm.data()[0]));
        },
        [&demuxer](const ff_cpp::Packet& pkt) {
          if (pkt.streamIndex() == demuxer.bestVideoStream().index()) {
            return true;
          }
          return false;
        });

    auto filteredFrm = filter.filter(inFrm);
    REQUIRE(std::equal(inFrm.data()[0], inFrm.data()[0] + imgSize, filteredFrm.data()[0]));

    std::ofstream of{"976x976_rgb24.out.data", std::ios::binary};
    of.write(reinterpret_cast<char*>(filteredFrm.data()[0]), imgSize);
    REQUIRE(of);
    of.close();
  }
}