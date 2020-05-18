#include <SDL2/SDL.h>
#include <ff_cpp/ff_exception.h>
#include <ff_cpp/ff_filter.h>

#include <chrono>
#include <fstream>
#include <iostream>

constexpr int TIMEOUT = 5;
constexpr int x_pos = 0;
constexpr int y_pos = 25;
const std::string PARAM_INPUT = "input";
const std::string PARAM_WIDTH = "w";
const std::string PARAM_HEIGHT = "h";
const std::string PARAM_FORMAT = "format";
const std::string PARAM_FILTER = "filter";
const std::string PARAM_MAX_WIDTH = "mw";
const std::string PARAM_MAX_HEIGHT = "mh";

struct Args {
  std::string input;
  int width;
  int height;
  std::string format;
  std::string filter;
  int maxWidth = 800;
  int maxHeight = 600;
};

Args parseArgs(int argc, char** argv) {
  Args arguments;
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
    arguments.input = args[PARAM_INPUT];
    args.erase(PARAM_INPUT);
  } else {
    // throw std::runtime_error{PARAM_INPUT + " arg is absent"};
  }
  if (args.find(PARAM_WIDTH) != args.end()) {
    arguments.width = std::stoi(args[PARAM_WIDTH]);
    args.erase(PARAM_WIDTH);
  } else {
    throw std::runtime_error{PARAM_WIDTH + " arg is absent"};
  }
  if (args.find(PARAM_HEIGHT) != args.end()) {
    arguments.height = std::stoi(args[PARAM_HEIGHT]);
    args.erase(PARAM_HEIGHT);
  } else {
    throw std::runtime_error{PARAM_HEIGHT + " arg is absent"};
  }
  if (args.find(PARAM_FORMAT) != args.end()) {
    arguments.format = args[PARAM_FORMAT];
    args.erase(PARAM_FORMAT);
  } else {
    throw std::runtime_error{PARAM_FORMAT + " arg is absent"};
  }
  if (args.find(PARAM_FILTER) != args.end()) {
    arguments.filter = args[PARAM_FILTER];
    args.erase(PARAM_FILTER);
  }
  if (args.find(PARAM_MAX_WIDTH) != args.end()) {
    arguments.maxWidth = std::stoi(args[PARAM_MAX_WIDTH]);
    args.erase(PARAM_MAX_WIDTH);
  }

  if (args.find(PARAM_MAX_HEIGHT) != args.end()) {
    arguments.maxHeight = std::stoi(args[PARAM_MAX_HEIGHT]);
    args.erase(PARAM_MAX_HEIGHT);
  }

  return arguments;
}

int main(int argc, char** argv) {
  Args args;
  try {
    args = parseArgs(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    std::cerr << "Usage: ff_player [input=url] <w=width> <h=height>"
                 "[mw=maxWndHeight] [mw=maxWndWidth]"
                 "<format=format> [filter=filter]"
              << std::endl;
    return EXIT_FAILURE;
  }

  try {
    av_log_set_level(AV_LOG_VERBOSE);
    auto format = av_get_pix_fmt(args.format.c_str());
    if (format == AV_PIX_FMT_NONE) {
      std::cerr << args.format << " is unsupported" << std::endl;
      return EXIT_FAILURE;
    }

    const std::string filterFormat = "format=pix_fmts=rgb24";
    if (args.filter.find(filterFormat) == std::string::npos) {
      args.filter.empty() ? args.filter = filterFormat
                          : args.filter += "," + filterFormat;
    }

    constexpr double resizeFactor = 1.2;
    int wndWidth = args.width;
    int wndHeight = args.height;
    while (wndWidth > args.maxWidth || wndHeight > args.maxHeight) {
      wndWidth = static_cast<int>(wndWidth / resizeFactor);
      wndHeight = static_cast<int>(wndHeight / resizeFactor);
    }

    if (wndWidth != args.width || wndHeight != args.height) {
      args.filter += ",scale=" + std::to_string(wndWidth) + ":" +
                     std::to_string(wndHeight);
    }

    ff_cpp::Filter filter{args.filter, args.width, args.height, format};

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
      return EXIT_FAILURE;
    }
    SDL_Window* win = SDL_CreateWindow("Raw viewer", x_pos, y_pos, wndWidth,
                                       wndHeight, SDL_WINDOW_OPENGL);
    if (win == nullptr) {
      std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return EXIT_FAILURE;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);
    if (ren == nullptr) {
      SDL_DestroyWindow(win);
      std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return EXIT_FAILURE;
    }
    SDL_Texture* sdlTexture =
        SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24,
                          SDL_TEXTUREACCESS_STREAMING, wndWidth, wndHeight);

    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = wndWidth;
    sdlRect.h = wndHeight;

    int imgSize = av_image_get_buffer_size(static_cast<AVPixelFormat>(format),
                                           args.width, args.height, 1);

    std::unique_ptr<char[]> img(new char[imgSize]);

    if (!args.input.empty()) {
      std::ifstream f{args.input, std::ios::binary};
      if (!f) {
        std::cerr << "Unable to open " << args.input << std::endl;
        return EXIT_FAILURE;
      }

      f.read(img.get(), imgSize);
      f.close();
    } else {
      auto bpl = imgSize / args.height;
      for (auto i = 0; i < imgSize; i++) {
        if (i % 10 == 0 || (i / bpl) % 10 == 0) {
          img[i] = 0;
        } else {
          img[i] = (char)255;
        }
      }
    }

    ff_cpp::Frame frame{reinterpret_cast<uint8_t*>(img.get()), args.width,
                        args.height, format, 1};
    auto startFiltering = std::chrono::steady_clock::now();
    auto filteredFrame = filter.filter(frame, true);
    std::cout << "Filtering took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now() - startFiltering)
                     .count()
              << "ms" << std::endl;

    SDL_Event e;
    bool quit = false;
    while (!quit) {
      SDL_UpdateTexture(sdlTexture, &sdlRect, filteredFrame.data()[0],
                        filteredFrame.linesize()[0]);
      SDL_RenderClear(ren);
      SDL_RenderCopy(ren, sdlTexture, nullptr, &sdlRect);
      SDL_RenderPresent(ren);

      while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
          quit = true;
        }
      }
    }

    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
  } catch (const std::exception& e) {
    std::cerr << "FF_CPP exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}