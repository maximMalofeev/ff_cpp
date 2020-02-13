from conans import ConanFile, CMake, tools


class FFCPPConan(ConanFile):
    name = "ff_cpp"
    version = "0.1"
    license = "proprietary"
    author = "Maxim Malofeev m.malofeev@recognize.ru"
    url = "https://github.com/maximMalofeev/ff_cpp.git"
    description = "ff_cpp"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = "ffmpeg/4.2.1@bincrafters/stable", "sdl2/2.0.10@bincrafters/stable"
    build_requires = "catch2/2.11.0"

    def source(self):
       self.run("git clone https://github.com/maximMalofeev/ff_cpp.git")
       # self.run("cd hello && git checkout static_shared")
       # This small hack might be useful to guarantee proper /MT /MD linkage
       # in MSVC if the packaged project doesn't have variables to set it
       # properly
#        tools.replace_in_file("hello/CMakeLists.txt", "PROJECT(MyHello)",
#                              '''PROJECT(MyHello)
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()''')

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="ff_cpp")
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="ff_cpp/include")
        self.copy("*ff_cpp.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")

