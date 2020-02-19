#include <SDL2\SDL.h>
#include <ff_cpp\ff_demuxer.h>
#include <ff_cpp\ff_exception.h>
#include <ff_cpp\ff_info.h>

#include <iostream>

int main(int argc, char** argv) {
  avdevice_register_all();

  AVInputFormat* iformat = av_find_input_format("");

  ff_cpp::Demuxer demuxer("video=USB2.0 UVC HQ WebCam", "dshow");
  try {
    demuxer.prepare({{"video_size","hd720"}});
    std::cout << demuxer << std::endl;

    auto& vStream = demuxer.bestVideoStream();

    if (vStream.format() != AV_PIX_FMT_YUYV422) {
      std::cerr << "Unsupported pixel format" << std::endl;
      return 1;
    }

    demuxer.createDecoder(vStream.index());

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
      return 1;
    }

    constexpr int x_pos = 100;
    constexpr int y_pos = 100;
    SDL_Window* win =
        SDL_CreateWindow("Hello World!", x_pos, y_pos, vStream.width(),
                         vStream.height(), SDL_WINDOW_OPENGL);
    if (win == nullptr) {
      std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return 1;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
    if (ren == nullptr) {
      SDL_DestroyWindow(win);
      std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return 1;
    }

    SDL_Texture* sdlTexture = SDL_CreateTexture(
        ren, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING, vStream.width(),
        vStream.height());

    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = vStream.width();
    sdlRect.h = vStream.height();

    demuxer.start(
        [&](const AVFrame* frm) {
          // std::cout << "Pts: " << frm->pts << std::endl;
          SDL_UpdateTexture(sdlTexture, &sdlRect, frm->data[0],
                            frm->linesize[0]);
          SDL_RenderClear(ren);
          SDL_RenderCopy(ren, sdlTexture, nullptr, &sdlRect);
          SDL_RenderPresent(ren);
        },
        [&demuxer](const AVPacket* pkt) {
          if (pkt->stream_index == demuxer.bestVideoStream().index()) {
            return true;
          }
          return false;
        });
  } catch (const ff_cpp::FFCppException& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}