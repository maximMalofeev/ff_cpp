#pragma once

#include <string>
#include <map>
#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#if defined(WIN32) && defined(FF_CPP_SHARED)
#if defined(FF_CPP)
#define FF_CPP_API __declspec(dllexport)
#else
#define FF_CPP_API __declspec(dllimport)
#endif
#else
#define FF_CPP_API
#endif

namespace ff_cpp {

static const std::string av_make_error_string(int errnum) {
  char errbuf[AV_ERROR_MAX_STRING_SIZE];
  av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
  return (std::string)errbuf;
}
#undef av_err2str 
#define av_err2str(errnum) av_make_error_string(errnum).c_str() 

using MetadataContainer = std::map<std::string, std::string>;
using ParametersContainer = MetadataContainer;

}