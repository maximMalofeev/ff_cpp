#pragma once
#include <ff_cpp/ff_include.h>

namespace ff_cpp {

class FFCppException : public std::runtime_error {
 public:
  FFCppException(const std::string& msg) : std::runtime_error(msg) {}
};

class BadInput : public FFCppException {
 public:
  BadInput(const std::string& msg, const std::string& url)
      : FFCppException(msg), url_(url) {}
  const std::string& url() const { return url_; }

 private:
  std::string url_;
};

class OptionsNotAccepted : public FFCppException {
 public:
  OptionsNotAccepted(const std::string& msg, ParametersContainer params)
      : FFCppException(msg), params_(params) {}
  const ParametersContainer& NotAcceptedOptions() const { return params_; }

 private:
  ParametersContainer params_;
};

class NoStream : public FFCppException {
 public:
  NoStream(const std::string& msg) : FFCppException(msg) {}
};

class NoDecoder : public FFCppException {
 public:
  NoDecoder(const std::string& msg, AVCodecID id)
      : FFCppException(msg), id_(id) {}
  AVCodecID id() const { return id_; }

 private:
  AVCodecID id_;
};

class ProcessingError : public FFCppException {
 public:
  ProcessingError(const std::string& msg) : FFCppException(msg) {}
};

}  // namespace ff_cpp