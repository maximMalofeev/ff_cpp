from conans import ConanFile, CMake, tools


class FFCPPConan(ConanFile):
    name = "ff_cpp"
    version = "0.5"
    license = ""
    author = "Maxim Malofeev maximmalofeev@bk.ru"
    url = "https://github.com/maximMalofeev/ff_cpp.git"
    description = "ff_cpp"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    requires = "ffmpeg/4.0@bincrafters/stable", "sdl2/2.0.9@bincrafters/stable", "catch2/2.12.2"

    def source(self):
       self.run("git clone https://github.com/maximMalofeev/ff_cpp.git")
       self.run("git checkout pkg")

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

    def package_info(self):
        self.cpp_info.libs = ["ff_cpp"]

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
