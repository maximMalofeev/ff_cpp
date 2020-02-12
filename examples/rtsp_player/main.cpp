#include <SDL2\SDL.h>
#include <ff_cpp\ff_demuxer.h>
#include <ff_cpp\ff_exception.h>
#include <iostream>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: rtsp_player <rtsp_url>" << std::endl;
    return 1;
  }
  
  ff_cpp::Demuxer demuxer(argv[1]);
  try {
    demuxer.prepare({{"fflags", "autobsf+discardcorrupt+genpts+ignidx+igndts"},
                     {"rtsp_transport", "tcp"},
                     {"allowed_media_types", "video"}},
                    5);

    std::cout << demuxer << std::endl;

    auto& vStream = demuxer.bestVideoStream();

    if (vStream.format() != AV_PIX_FMT_YUV420P) {
      std::cerr << "Unsupported pixel format" << std::endl;
      return 1;
    }

    demuxer.createDecoder(vStream.index());

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
      return 1;
    }

    SDL_Window* win =
        SDL_CreateWindow("Hello World!", 100, 100, vStream.width(),
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
        ren, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, vStream.width(),
        vStream.height());

    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = vStream.width();
    sdlRect.h = vStream.height();

    demuxer.start(
        [&](const AVFrame* frm) {
          SDL_UpdateYUVTexture(sdlTexture, &sdlRect, frm->data[0],
                               frm->linesize[0], frm->data[1], frm->linesize[1],
                               frm->data[2], frm->linesize[2]);
          SDL_RenderClear(ren);
          SDL_RenderCopy(ren, sdlTexture, NULL, &sdlRect);
          SDL_RenderPresent(ren);
        },
        [&demuxer](const AVPacket* pkt) {
          if (pkt->stream_index == demuxer.bestVideoStream().index()) {
            return true;
          }
          return false;
        });

  } catch (const ff_cpp::FFCppException& e) {
    std::cerr << "FF_CPP exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}