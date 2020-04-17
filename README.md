<p align="center">
  <img align="center" width="128" height="128" src="https://avatars1.githubusercontent.com/u/16670651?s=128&v=4">
</p>

<h1 align="center">enve</h1>

Enve is a new open-source 2D animation software for Linux and Windows.
You can use enve to create vector animations, raster animations, and even use sound and video files.
Enve was created with flexibility and expandability in mind.

<p align="center">
<a href="https://twitter.com/enve2d" target="_blank">
  <img src="https://image.flaticon.com/icons/png/512/187/187187.png" alt="enve on Twitter" height="64" border="10" hspace="20"/>
</a>
&nbsp;&nbsp;&nbsp;
<a href="https://www.youtube.com/channel/UCjvNO3xdIQs-cpgwSf0mX5Q" target="_blank">
  <img src="https://image.flaticon.com/icons/png/512/187/187209.png" alt="enve on YouTube" height="64" border="10" hspace="20"/>
</a>
&nbsp;&nbsp;&nbsp;
<a href="https://www.patreon.com/enve" target="_blank">
  <img src="https://is3-ssl.mzstatic.com/image/thumb/Purple128/v4/9e/f2/81/9ef281df-1da2-e183-18d2-6c475965fef8/AppIcon-0-1x_U007emarketing-0-0-GLES2_U002c0-512MB-sRGB-0-0-0-85-220-0-0-0-7.png/246x0w.jpg" alt="enve on Patreon" height="64" border="10" hspace="20"/>
</a>
&nbsp;&nbsp;&nbsp;
<a href="https://liberapay.com/enve" target="_blank">
  <img src="https://liberapay.com/assets/liberapay/icon-v2_white-on-yellow.svg?etag=.Z1LYSBJ8Z6GWUeLUUEf2XA~~" alt="enve on Liberapay" height="64" border="10" hspace="20"/>
</a>
&nbsp;&nbsp;&nbsp;
<a href="https://paypal.me/enve2d" target="_blank">
  <img src="https://www.paypalobjects.com/webstatic/mktg/logo/pp_cc_mark_111x69.jpg" alt="enve on PayPal" height="64" border="10" hspace="20"/>
</a>
</p><br/>

<img src="https://user-images.githubusercontent.com/16670651/70745938-36e20900-1d25-11ea-9bdf-78d3fe402291.png"/>

## Download
You can download the latest enve release for <a href="https://github.com/MaurycyLiebner/enve/releases/continuous-linux/" target="_blank">Linux</a> and <a href="https://github.com/MaurycyLiebner/enve/releases/continuous-windows/" target="_blank">Windows</a>.

## Get Enve Source

Make sure you have **git** installed on your system.

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
Initialize and checkout all submodules:
```
git submodule update --init
```
Apply enve-specific library patches (not needed for Windows):
```
make patch
```

<h1 align="center">Building for Linux</h1>

Some of the provided terminal commands are applicable only to Ubuntu 16.04 and 18.04.
If you are using a different distribution please treat the commands as a general guide.

### Skia

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
bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] target_os="linux" target_cpu="x64" skia_use_system_expat=false skia_use_system_freetype2=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_icu=false cc="gcc-7" cxx="g++-7"'
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

### Libmypaint
Install libmypaint dependencies:
```
sudo apt-get install libjson-c-dev intltool pkg-config
```
Enter libmypaint directory:
```
cd libmypaint
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

### Gperftools
Install packages needed to build gperftools:
```
sudo apt-get install autoconf automake libtool
sudo apt-get install libunwind-dev
```

Build gperftools:
```
cd gperftools
./autogen.sh
./configure --prefix /usr
make
```
Go back to the third_party folder:
```
cd ..
```
### Qt

Go to qt.io/download to download Open Source Qt installer.
Install Qt 5.12.4 Desktop gcc 64-bit.

Install Qt in the directory of your choice (ex. ~/.Qt)

### QScintilla
Build QScintilla:
```
cd qscintilla/Qt4Qt5
qmake
make -j 2
```
Go back to the third_party folder:
```
cd ..
```
### Quazip

```
cd quazip\quazip
qmake INCLUDEPATH+=$$[QT_INSTALL_HEADERS]/QtZlib
make -j 2
```

Go back to the main enve directory:
```
cd ../../../
```

### FFmpeg
Install libraries needed for audio/video decoding/encoding.
```
sudo add-apt-repository ppa:jonathonf/ffmpeg-4
sudo apt-get update
sudo apt-get install libswresample-dev libswscale-dev libavcodec-dev libavformat-dev libavresample-dev
```

### Other
enve dependencies:
```
sudo apt-get install libglib2.0-dev
```
Install libxkbcommon-x11-dev to run QtCreator on Ubuntu 16.04.
Otherwise it will not execute properly.
```
sudo apt-get install libxkbcommon-x11-dev
```

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

## Deploying for Linux

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

<h1 align="center">Building for Windows</h1>

### Visual Studio Community 2017

Install Visual Studio Community 2017 from **https://visualstudio.microsoft.com/vs/older-downloads**.	
Make sure to install **Desktop development with C++**

### Qt

Go to qt.io/download to download Open Source Qt installer.
Install Qt 5.12.4 Desktop MSVC 2017 64-bit.

### LLVM

Download the latest stable LLVM relese from **https://github.com/llvm/llvm-project/releases**, e.g., **LLVM-10.0.0-win64.exe**.
Install LLVM.

### Environment

Run all commands from **Qt 5.12.4 (MSVC 2017 64-bit)** command line.
Remeber to call vcvarsall.bat to initialize MSVC environment, e.g.,
`"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64`.

### FFmpeg and Libmypaint

Use batch script to download prebuilt libraries:
```
cd third_party
win.bat
```

### QScintilla

```
cd qscintilla\Qt4Qt5
qmake CONFIG-=debug_and_release
nmake
```

### Quazip

```
cd quazip\quazip
qmake CONFIG-=debug_and_release INCLUDEPATH+=$$[QT_INSTALL_HEADERS]/QtZlib
nmake
```

### Skia

Enter Skia directory:

```
cd skia
```

```
python tools/git-sync-deps
```

Configure the release build:

```
bin\gn.exe gen out/Release --args="is_official_build=true is_debug=false extra_cflags=[\"-Wno-error\",\"/MD\",\"/O2\"] target_os=\"windows\" host_os=\"win\" current_os=\"win\" target_cpu=\"x64\" clang_win=\"C:\Program Files\LLVM\" cc=\"clang-cl\" cxx=\"clang-cl\" skia_use_system_expat=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false" --ide=vs
```

Enter the build directory:

```
cd out\Release
```

Build Skia:

```
msbuild all.sln
```

### enve

```
qmake enve.pro -spec win32-clang-msvc -config release
nmake
```

<h1 align="center">Building for macOS</h1>

Install build dependencies for third-party libraries from Homebrew.
```sh
brew install ninja json-c intltool pkg-config gettext zlib ffmpeg
```

### Skia

```sh
pyenv shell system  # disable pyenv as build script breaks under Python 3
tools/git-sync-deps
bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] skia_use_system_expat=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false'
ninja -C out/Release skia
```

### Libmypaint

```sh
# Manually specify environmental variables for keg-only dependencies.
ACLOCAL_FLAGS="-I/usr/local/opt/gettext/share/aclocal $ACLOCAL_FLAGS"
LDFLAGS="-L/usr/local/opt/gettext/lib $LDFLAGS"
CPPFLAGS="-I/usr/local/opt/gettext/include $CPPFLAGS"
PATH="/usr/local/opt/gettext/bin:$PATH"
./autogen.sh
./configure --enable-openmp --prefix=/usr/local
make
ln -s `pwd` libmypaint
```

### Quazip

```sh
# Explicitly add zlib to path upon build, as Homebrew zlib is keg-only.
# Do not `brew link zlib` as it might conflict with the stock version shipped with macOS.
LDFLAGS="-L/usr/local/opt/zlib/lib $LDFLAGS"
CPPFLAGS="-I/usr/local/opt/zlib/include $CPPFLAGS"
qmake quazip.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 LIBS+=-lz
make

```

### Gperftools

```sh
CFLAGS="$CFLAGS -Wno-error -D_XOPEN_SOURCE"
./autogen.sh
./configure --disable-dependency-tracking --prefix=/usr/local
make
```

### QScintilla

```sh
cd Qt4Qt5
qmake -spec macx-clang CONFIG+=release
```

### enve

```
qmake enve.pro -spec macx-clang CONFIG+=release
make
```

## Authors

**Maurycy Liebner** - 2016 - 2020 - [MaurycyLiebner](https://github.com/MaurycyLiebner)

## License

This project is licensed under the GPL3 License - see the [LICENSE.md](LICENSE.md) file for details
