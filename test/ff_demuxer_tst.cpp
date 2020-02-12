#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
                           // this in one cpp file
#include <ff_cpp/ff_demuxer.h>
#include <ff_cpp/ff_exception.h>
#include <catch2/catch.hpp>

const std::string url = "file:small_bunny_1080p_60fps.mp4";
const std::string emptyFileUrl = "file:empty_file.mp4";

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
    REQUIRE_THROWS_AS(demuxer.prepare({{"invalid", "parameter"}}), ff_cpp::OptionsNotAccepted);
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
    REQUIRE(demuxer.streams().size() == 0);
    REQUIRE_THROWS_AS(demuxer.bestVideoStream(), ff_cpp::FFCppException);
  }
  SECTION("Prepared demuxer has have at least 1 stream") {
    ff_cpp::Demuxer demuxer(url);
    demuxer.prepare();
    REQUIRE(demuxer.streams().size() != 0);
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
    REQUIRE(bestVStream.averageFPS() == 60);
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
        [&demuxer](const AVFrame* frm) {
          REQUIRE(frm->width == 1920);
          REQUIRE(frm->height == 1080);
          REQUIRE(frm->format ==
                  static_cast<AVPixelFormat>(AV_PIX_FMT_YUV420P));
          demuxer.stop();
        },
        [&demuxer](const AVPacket* pkt) {
          if (pkt->stream_index == demuxer.bestVideoStream().index()) {
            return true;
          }
          return false;
        });
    SUCCEED();
  }
}