#pragma once
#include <ff_cpp/ff_include.h>
#include <ff_cpp/ff_frame.h>

namespace ff_cpp {

//TODO add getters
class Scaler {
 public:
  /**
   * @brief Construct a new Scaler object that change only pixel format
   * 
   * @param width - src and dst frame width
   * @param height - src and dst frame height
   * @param srcFormat - src frame pixel format
   * @param dstFormat - dst frame pixel format
   * @exception FFCppException - in case of no ability to create scaler
   */
  //TODO add constructor to change dst width and height
  FF_CPP_API Scaler(int width, int height, AVPixelFormat srcFormat,
         AVPixelFormat dstFormat);
  FF_CPP_API ~Scaler();

  /**
   * @brief Scale source frame and return new scaled frame
   * 
   * @param srcFrame - source frame
   * @param dstAlignment - alignment for returned frame
   * @return new ff_cpp::Frame
   * @exception FFCppException - in case of no ability to scale or if input args wrong
   */
  FF_CPP_API ff_cpp::Frame scale(ff_cpp::Frame& srcFrame, int dstAlignment);
  /**
   * @brief Scale source frame into destination frame
   * 
   * @param srcFrame - source frame
   * @param dstFrame - destination frame
   * @return reference to destination frame
   * @exception FFCppException - in case of no ability to scale or if input args wrong
   */
  FF_CPP_API ff_cpp::Frame& scale(ff_cpp::Frame& srcFrame, ff_cpp::Frame& dstFrame);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}