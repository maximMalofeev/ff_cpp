#pragma once
#include <ff_cpp/ff_include.h>

#include <array>

namespace ff_cpp {

//TODO add copyToBuffer function
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
   * @param align - the value to use for buffer size alignment
   * @throw FFCppException in case of wrong input params or in case of memory alloc failed
   */
  FF_CPP_API Frame(int width, int height, int format, int align = 1);
  /**
   * @brief Create frame with input buffer with specified parameters
   * @note input buffer must exists until frame not destroyed
   * 
   * @param ptr - input buffer
   * @param width - buffer width
   * @param height - buffer height
   * @param format - buffer format
   * @param align - the value to use for buffer size alignment
   * @throw FFCppException in case of wrong input params or in case of memory alloc failed
   */
  //TODO: add constructor with deleater that will take ownership of the ptr
  FF_CPP_API Frame(const uint8_t* ptr, int width, int height, int format, int align = 1);
  FF_CPP_API Frame(Frame&& other);
  FF_CPP_API ~Frame();

  FF_CPP_API int width() const;
  FF_CPP_API int height() const;
  FF_CPP_API int format() const;

  FF_CPP_API int64_t pts() const;
  FF_CPP_API void setPts(int64_t pts);
  FF_CPP_API int64_t dts() const;
  FF_CPP_API void setDts(int64_t dts);

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
