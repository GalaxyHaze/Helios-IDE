class Helios < Formula
  desc "Qt desktop IDE for Zith projects"
  homepage "https://github.com/GalaxyHaze/Helios-IDE"
  url "https://github.com/GalaxyHaze/Helios-IDE/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "REPLACE_WITH_RELEASE_SOURCE_SHA256"
  head "https://github.com/GalaxyHaze/Helios-IDE.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "ninja" => :build
  depends_on "qt"

  def install
    qt_prefix = Formula["qt"].opt_prefix

    system "cmake", "-S", ".", "-B", "build", "-G", "Ninja",
                    *std_cmake_args,
                    "-DCMAKE_PREFIX_PATH=#{qt_prefix}"
    system "cmake", "--build", "build"
    bin.install "build/Helios"
  end
end
