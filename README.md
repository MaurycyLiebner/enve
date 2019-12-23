<p align="center">
  <img align="center" width="128" height="128" src="https://avatars1.githubusercontent.com/u/16670651?s=128&v=4">
</p>

<h1 align="center">enve</h1>

Enve is a new open-source 2D animation software for Linux.
You can use enve to create vector animations, raster animations, and even use sound and video files.
Enve was created with flexibility and expandability in mind.

<p align="center">
  <a href="https://twitter.com/enve2d" target="_blank">
    <img src="https://image.flaticon.com/icons/png/512/187/187187.png" alt="enve on Twitter" width="64" height="64" border="10" hspace="20"/>
  </a>

  <a href="https://www.youtube.com/channel/UCjvNO3xdIQs-cpgwSf0mX5Q" target="_blank">
    <img src="https://image.flaticon.com/icons/png/512/187/187209.png" alt="enve on YouTube" width="64" height="64" border="10" hspace="20"/>
  </a>

  <a href="https://www.patreon.com/enve" target="_blank">
    <img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" alt="enve on Patreon" height="64" border="10" hspace="20"/>
  </a>
</p>

![screenshot_1](https://user-images.githubusercontent.com/16670651/70745938-36e20900-1d25-11ea-9bdf-78d3fe402291.png)

## How to build

Some of the provided terminal commands are applicable only to Ubuntu 16.04 and 18.04.
If you are using a different distribution please treat the commands as a general guide.

### Dependencies

Install git:
```
sudo apt-get install git
```

Clone the enve repository:
```
git clone https://github.com/MaurycyLiebner/enve
```
Enter enve directory:
```
cd enve
```
Enter the folder with third party dependencies:
```
cd third_party
```
#### Skia

Extract skia:
```
tar xf skia.tar.xz
```
Enter the skia directory:
```
cd skia
```
Synchronize the skia dependencies:
```
python tools/git-sync-deps
```
Install the skia dependencies:
```
tools/install_dependencies.sh
```
Install g++-7:
```
sudo add-apt-repository ppa:jonathonf/gcc
sudo apt-get update
sudo apt-get install g++-7
```
Download ninja, extract, and set permission for the executable:
```
wget https://github.com/ninja-build/ninja/releases/download/v1.9.0/ninja-linux.zip
sudo apt-get install unzip
unzip ninja-linux.zip
chmod +x ninja
```
Create the release build (ninja) files:
```
bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] target_os="linux" target_cpu="x64" skia_use_system_expat=false skia_use_system_freetype2=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false cc="gcc-7" cxx="g++-7"'
```
Build a release version of skia (you can use more than 2 threads):
```
./ninja -C out/Release -j 2
```
If you want, you can also build a debug version:
```
bin/gn gen out/Debug --args='extra_cflags=["-Wno-error"] cc="gcc-7" cxx="g++-7"'
./ninja -C out/Debug -j 2
```

Go back to the third_party folder:
```
cd ..
```

#### Libmypaint
Install libmypaint dependencies:
```
sudo apt-get install libjson-c-dev intltool pkg-config
```
Extract libmypaint:
```
tar xf libmypaint.tar.xz
```
Enter libmypaint directory:
```
cd libmypaint-1.3.0
```
Set CFLAGS for better optimization:
```
export CFLAGS='-fopenmp -Ofast -ftree-vectorize -fopt-info-vec-optimized -funsafe-math-optimizations -funsafe-loop-optimizations -fPIC'
```
Configure libmypaint:
```
./configure --enable-static --enable-openmp --enable-shared=false
```
Build libmypaint:
```
sudo make
```
Go back to the third_party folder:
```
cd ..
```

#### gperftools
Install packages needed to build gperftools:
```
sudo apt-get install autoconf automake libtool
sudo apt-get install libunwind-dev
```

Build gperftools:
```
cd gperftools-2.7-enve-mod
./autogen.sh
./configure --prefix /usr
make
```

Go back to the main enve directory:
```
cd ../../
```

#### FFmpeg
Install libraries needed for audio/video decoding/encoding.
```
sudo add-apt-repository ppa:jonathonf/ffmpeg-4
sudo apt-get update
sudo apt-get install libswresample-dev libswscale-dev libavcodec-dev libavformat-dev libavresample-dev
```

#### Other
enve dependencies:
```
sudo apt-get install libglib2.0-dev
```
Install libxkbcommon-x11-dev to run QtCreator on Ubuntu 16.04.
Otherwise it will not execute properly.
```
sudo apt-get install libxkbcommon-x11-dev
```

#### Qt

Go to qt.io/download to download Open Source Qt installer.
Install Qt 5.12.4 Desktop gcc 64-bit.

Install Qt in the directory of your choice (ex. ~/.Qt)

### enve itself

You can either build enve through QtCreator (open enve.pro and setup Relase/Debug Kits),
or by running qmake and make directly.

By default qmake is not installed in /usr/bin.
You will have to call it by its full path or create a symbolic link:
```
sudo ln -s your_Qt_dir/5.12.4/gcc_64/bin/qmake /usr/bin/qmake
```

Build a release version of enve:
```
cd build/Release
qmake ../../enve.pro
make CC=gcc-7 CPP=g++-7 CXX=g++-7 LD=g++-7
cd ..
```
Build a debug version of enve (if needed):
```
cd Debug
qmake CONFIG+=debug ../../enve.pro
make CC=gcc-7 CPP=g++-7 CXX=g++-7 LD=g++-7
cd ..
```
Now you have successuly build enve and libenvecore along with all the examples.
If you wish to create your own executable proceed to the **Deployment** section.

## Deployment

Make sure you are in the build directory.

### Download needed tools

Download and change permission for LinuxDeployQt:
```
wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
chmod +x linuxdeployqt-6-x86_64.AppImage
```
Copy the enve build into the AppDir:
```
cp Release/src/app/enve AppDir/usr/bin/
cp -av Release/src/core/*.so* AppDir/usr/lib/
```

#### Support older systems (ex. Ubuntu 16.04)
Download patched AppRun binary and an associated library that will link a newer version of libstdc++ on older systems:
```
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64
mv AppRun-patched-x86_64 AppRun
chmod +x AppRun
cp AppRun AppDir/
```
```
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so
mv exec-x86_64.so exec.so
cp exec.so AppDir/usr/optional
```
Copy your system's version of libstdc++ to your AppDir:
```
mkdir AppDir/usr/optional/libstdc++
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/optional/libstdc++/
```

#### Create AppImage

Create the AppImage using LinuxDeployQt:
```
./linuxdeployqt-6-x86_64.AppImage AppDir/usr/share/applications/enve.desktop -appimage
```

You have successfuly created your own enve AppImage!

## Authors

**Maurycy Liebner** - 2016 - 2019 - [MaurycyLiebner](https://github.com/MaurycyLiebner)

## License

This project is licensed under the GPL3 License - see the [LICENSE.md](LICENSE.md) file for details
