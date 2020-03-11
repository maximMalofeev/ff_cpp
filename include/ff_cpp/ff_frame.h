#pragma once
#include <ff_cpp/ff_include.h>

#include <array>

namespace ff_cpp {

class Frame {
 public:
  /**
   * @brief Default frame, image buffer not allocated
   */
  FF_CPP_API Frame();
  /**
   * @brief Create frame and allocate image accordingly input params
   * 
   * @param width - frame width
   * @param height - frame height
   * @param format - frame format
   */
  FF_CPP_API Frame(int width, int height, int format);
  /**
   * @brief Create frame using input buffer with specified parameters
   * 
   * @param ptr - input buffer
   * @param width - buffer width
   * @param height - buffer height
   * @param format - buffer format
   */
  FF_CPP_API Frame(uint8_t* ptr, int width, int height, int format);
  FF_CPP_API Frame(Frame&& other);
  FF_CPP_API ~Frame();

  FF_CPP_API int width() const;
  FF_CPP_API int height() const;
  FF_CPP_API int format() const;

  FF_CPP_API int64_t pts() const;
  FF_CPP_API int64_t dts() const;

  /**
   * @brief Return number of data pointers, it uses for data and linesize
   * 
   * @return numDataPointers 
   */
  FF_CPP_API int numDataPointers() const;
  /**
   * @brief return image data
   * 
   * @return FF_CPP_API** data 
   */
  FF_CPP_API uint8_t** data() const;
  /**
   * @brief return image linesizes
   * 
   * @return FF_CPP_API* linesize 
   */
  FF_CPP_API int* linesize() const;

  friend std::ostream& operator<<(std::ostream& ost, const Frame& frame);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;

  friend class Decoder;
  friend class Filter;
  operator AVFrame*();
};

}  // namespace ff_cpp
