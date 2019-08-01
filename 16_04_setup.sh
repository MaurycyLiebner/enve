#!/bin/sh -e
echo -e "The following script downloads, builds and installs everything that is required to build enve.\nInternet connection is required.\nIt was created for clean Ubuntu 16.04 installations.\nIf you do not have a clean Ubuntu 16.04 installation you can open the script as a text file and treat it as a guide for what you have to do to get started.\nDo you want to continue (y/n)? "
read answer
if [ "$answer" == "${answer#[Yy]}" ]; then
    exit 0
fi

cd third_party/
tar xf skia.tar.xz
cd skia

echo -e "\n---Install Skia dependencies---\n"

sudo apt-get -y install git wget
sudo add-apt-repository -y ppa:jonathonf/gcc
sudo apt-get update
sudo apt-get -y install g++-7
sudo add-apt-repository -y --remove ppa:jonathonf/gcc
sudo apt-get update
python tools/git-sync-deps
tools/install_dependencies.sh
wget https://github.com/ninja-build/ninja/releases/download/v1.9.0/ninja-linux.zip
sudo apt-get -y install unzip
unzip ninja-linux.zip
chmod +x ninja

echo -e "\n---Skia dependencies installed---\n"

bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] target_os="linux" target_cpu="x64" skia_use_system_expat=false skia_use_system_freetype2=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false cc="gcc-7" cxx="g++-7"'
./ninja -C out/Release -j 2
cd ../

echo -e "\n---Install libmypaint dependencies---\n"

sudo apt-get -y install libjson-c-dev intltool pkg-config

echo -e "\n---Libmypaint dependencies installed---\n"

tar xf libmypaint.tar.xz
cd libmypaint-1.3.0
export CFLAGS='-fopenmp -Ofast -ftree-vectorize -fopt-info-vec-optimized -funsafe-math-optimizations -funsafe-loop-optimizations -fPIC'
./configure --enable-static --enable-openmp --enable-shared=false
sudo make
cd ../../

echo -e "\n---Finished libmypaint---\n"

echo -e "\n---Install enve dependencies---\n"

sudo add-apt-repository -y ppa:jonathonf/ffmpeg-4
sudo apt-get update
sudo apt-get -y install libswresample-dev libswscale-dev libavcodec-dev libavformat-dev libavresample-dev
sudo add-apt-repository -y --remove ppa:jonathonf/ffmpeg-4
sudo apt-get update
sudo apt-get -y install libgoogle-perftools-dev
sudo apt-get -y install libglib2.0-dev
sudo apt-get -y install libxkbcommon-x11-dev

echo -e "\n---enve dependencies installed---\n"

echo -e "\n---Download AppImageKit and LinuxDeployQt---\n"

cd build/
wget https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage
mv appimagetool-x86_64.AppImage appimagetool.AppImage
chmod +x appimagetool.AppImage

wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
mv linuxdeployqt-6-x86_64.AppImage linuxdeployqt.AppImage
chmod +x linuxdeployqt.AppImage

wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so
mv exec-x86_64.so exec.so

wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64
mv AppRun-patched-x86_64 AppRun
chmod +x AppRun

echo -e "\n---Download Qt---\n"

cd ~/Downloads
wget http://download.qt.io/official_releases/qt/5.12/5.12.4/qt-opensource-linux-x64-5.12.4.run

echo -e "\n---Qt downloaded---\n"
echo -e "\n---Install Qt---\n"

chmod +x qt-opensource-linux-x64-5.12.4.run
./qt-opensource-linux-x64-5.12.4.run
echo -e "\n---Enter Qt folder path: (leave empty for '~/.Qt')---\n"
read dirpath
if [ -z "$dirpath" ]; then
	dirpath = "~/.Qt"
fi

if [ -d "$DIRECTORY" ]; then
	sudo ln "${dirpath}/5.12.4/gcc_64/bin/qmake" /usr/bin/qmake
else
	echo -e "\n---Invalid directory, please create a link to qmake binary in '/usr/bin/'---\n"
fi

echo -e "\n---All finished---\n"