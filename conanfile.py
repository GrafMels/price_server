import os
from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout, CMakeDeps
from conan.tools.files import rmdir


class PriceServerRecipe(ConanFile):
    name = "price_server"
    version = "1.0.0"
    description = ""
    author = "mels <ilia2577@gmail.com>"
    topics = ("qt")

    settings = "os", "arch", "compiler", "build_type"

    exports_sources = "*"

    options = {
        "shared": [True, False],
        "standalone": [True, False]
    }
    default_options = {
        "shared": False,
        "standalone": False
    }

    @property
    def _min_cppstd(self):
        return "20"

    def requirements(self):
        self.requires("spdlog/1.15.3")
        self.requires("grpc/1.72.0")
        self.requires("asio/1.34.2")

    def layout(self):
        cmake_layout(self)

    def recursive_copying(self, main, folder):
        for dep in main.dependencies.values():
            bin_extensions = [".exe", ".dll", ".dylib", ".so.*", ".so", ".a"]
            self.recursive_copying(dep, folder)
            for ext in bin_extensions:
                try:
                    copy(main, f"*{ext}", dep.cpp_info.bindirs[0], folder + "/bin")
                    copy(main, f"*{ext}", dep.cpp_info.libdirs[0], folder + "/lib")
                except Exception as e:
                    print(f"failed to copy {dep.ref.name} binaries: {e}")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self, generator="Ninja")
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "res"))
        rmdir(self, os.path.join(self.package_folder, "share"))
