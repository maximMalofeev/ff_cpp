#include <SDL2/SDL.h>
#include <ff_cpp/ff_demuxer.h>
#include <ff_cpp/ff_exception.h>
#include <ff_cpp/ff_filter.h>

#include <iostream>
#include <mutex>
#include <thread>

constexpr int TIMEOUT = 5;
constexpr int x_pos = 20;
constexpr int y_pos = 40;
const std::string PARAM_INPUT = "input";
const std::string PARAM_FORMAT = "format";
const std::string PARAM_FILTER = "filter";

struct Args {
  std::string input;
  std::string format;
  std::string filter;
  std::map<std::string, std::string> demuxerParams;
};

Args parseArgs(int argc, char** argv) {
  Args argumets;
  std::map<std::string, std::string> args;
  for (int i = 1; i < argc; i++) {
    std::string arg{argv[i]};
    auto separatorPos = arg.find("=");
    if (separatorPos == std::string::npos || separatorPos == 0) {
      throw std::runtime_error{"Wrong parameter: " + arg};
    }

    auto param = arg.substr(0, separatorPos);
    auto value = arg.substr(separatorPos + 1, arg.size());
    args[param] = value;
  }

  if (args.find(PARAM_INPUT) != args.end()) {
    argumets.input = args[PARAM_INPUT];
    args.erase(PARAM_INPUT);
  } else {
    throw std::runtime_error{PARAM_INPUT + " arg is absent"};
  }

  if (args.find(PARAM_FORMAT) != args.end()) {
    argumets.format = args[PARAM_FORMAT];
    args.erase(PARAM_FORMAT);
  }

  if (args.find(PARAM_FILTER) != args.end()) {
    argumets.filter = args[PARAM_FILTER];
    args.erase(PARAM_FILTER);
  }

  argumets.demuxerParams = args;

  return argumets;
}

int main(int argc, char** argv) {
  Args args;
  try {
    args = parseArgs(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    std::cerr << "Usage: ff_player <input=url> [format=format] [filter=filter] "
                 "[[demuxerParam=param]..]"
              << std::endl;
    return 1;
  }

  ff_cpp::Demuxer demuxer(args.input, args.format);
  try {
    demuxer.prepare(args.demuxerParams, TIMEOUT);
    std::cout << demuxer << std::endl;

    auto& vStream = demuxer.bestVideoStream();
    demuxer.createDecoder(vStream.index());

    const std::string filterFormat = "format=pix_fmts=yuv420p";
    if (args.filter.find(filterFormat) == std::string ::npos) {
      args.filter.empty() ? args.filter = filterFormat
                          : args.filter += "," + filterFormat;
    }

    constexpr int maxW = 800;
    constexpr int maxH = 600;
    constexpr double resizeFactor = 1.2;
    int wndWidth = vStream.width();
    int wndHeight = vStream.height();
    while (wndWidth > maxW || wndHeight > maxH) {
      wndWidth = static_cast<int>(wndWidth / resizeFactor);
      wndHeight = static_cast<int>(wndHeight / resizeFactor);
    }

    if (wndWidth != vStream.width() || wndHeight != vStream.height()) {
      args.filter += ",scale=" + std::to_string(wndWidth) + ":" +
                     std::to_string(wndHeight);
    }

    ff_cpp::Filter filter{args.filter, vStream.width(), vStream.height(),
                          vStream.format()};

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
      return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Hello World!", x_pos, y_pos, wndWidth,
                                       wndHeight, SDL_WINDOW_OPENGL);
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

    SDL_Texture* sdlTexture =
        SDL_CreateTexture(ren, SDL_PIXELFORMAT_IYUV,
                          SDL_TEXTUREACCESS_STREAMING, wndWidth, wndHeight);

    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = wndWidth;
    sdlRect.h = wndHeight;

    bool quit = false;
    std::mutex sdlMutex;
    std::thread demuxerThread{
        [&]() {
          try {
            demuxer.start(
                [&](ff_cpp::Frame& frm) {
                  std::cout << "Pts: "
                            << frm.pts() *
                                   av_q2d(demuxer.bestVideoStream().timeBase())
                            << std::endl;

                  auto startFiltering = std::chrono::steady_clock::now();
                  auto filteredFrm = filter.filter(frm);
                  std::cout
                      << "Filtering took "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - startFiltering)
                             .count()
                      << "ms" << std::endl;

                  std::lock_guard<std::mutex> lg{sdlMutex};
                  SDL_UpdateYUVTexture(
                      sdlTexture, &sdlRect, filteredFrm.data()[0],
                      filteredFrm.linesize()[0], filteredFrm.data()[1],
                      filteredFrm.linesize()[1], filteredFrm.data()[2],
                      filteredFrm.linesize()[2]);
                  SDL_RenderClear(ren);
                  SDL_RenderCopy(ren, sdlTexture, nullptr, &sdlRect);
                  SDL_RenderPresent(ren);
                },
                [&demuxer](ff_cpp::Packet& pkt) {
                  if (pkt.streamIndex() == demuxer.bestVideoStream().index()) {
                    return true;
                  }
                  return false;
                });
          } catch (const ff_cpp::EndOfFile& e) {
            std::cout << e.what() << std::endl;
          } catch (const ff_cpp::FFCppException& e) {
            std::cerr << e.what() << std::endl;
            quit = true;
          }
        }};

    SDL_Event e;
    while (!quit) {
      while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
          std::lock_guard<std::mutex> lg{sdlMutex};
          demuxer.stop();
          quit = true;
        }
      }
    }

    demuxerThread.join();
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
  } catch (const ff_cpp::FFCppException& e) {
    std::cerr << "FF_CPP exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}