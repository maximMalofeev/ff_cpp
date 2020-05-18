#pragma once
#include <ff_cpp/ff_frame.h>
#include <ff_cpp/ff_include.h>

namespace ff_cpp {

class Filter {
 public:
  /**
   * @brief Filter constructor. width, height and format are parameters for
   * buffer source, allowedFormats - parameter for sink. AllowedFormats could be
   * empty, in that case format of filtered frame could be any. It able to
   * control format of filtered frame using 'format' filter, for example -
   * 'format=pix_fmts=yuv420p'
   *
   * @param filterDescr - filter string representation
   * @param width - input frame width
   * @param height - input frame height
   * @param format - input frame format
   * @param allowedFormats - list of allowed output formats
   * @param autoConvert - allow filter graph convert format
   *
   * @throw FFCppException in case of common errors, like alloc errors
   * @throw FilterError in case of wrong input parameters, or wrong filter
   * description
   */
  FF_CPP_API Filter(const std::string& filterDescr, int width, int height,
                    int format, const std::vector<int>& allowedFormats = {},
                    bool autoConvert = true);
  FF_CPP_API Filter(Filter&& other);
  FF_CPP_API ~Filter();

  /**
   * @brief This function returns filter description
   *
   * @return FF_CPP_API const& filterDescription
   */
  FF_CPP_API const std::string& filterDescription() const;
  /**
   * @brief Filter input frame
   *
   * @param frm - input frame
   * @param keepRef - if false and if the frame is reference-counted Filter will
   * take ownership of the reference(s) and reset the frm; if true - if the
   * frame if reference-counted, create a new reference, otherwise copy the
   * frame data.
   * @return filtered frame
   * @throw ProcessingError - unable to add input frame to buffer filter,
   * or unable to get filtered frame from sink
   */
  FF_CPP_API Frame filter(Frame& frm, bool keepRef = false);

  FF_CPP_API Filter& operator=(Filter&& other);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

  Filter(const Filter& other) = delete;
  Filter& operator=(const Filter& other) = delete;
};

}  // namespace ff_cpp