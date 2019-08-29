# enve

Flexible, user expandable 2D animation software.

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
bin/gn gen out/Release --args='
	is_official_build=true is_debug=false
	extra_cflags=["-Wno-error"] target_os="linux"
	target_cpu="x64" skia_use_system_expat=false
	skia_use_system_freetype2=false
	skia_use_system_libjpeg_turbo=false
	skia_use_system_libpng=false
	skia_use_system_libwebp=false
	skia_use_system_zlib=false
	skia_use_system_harfbuzz=false
	cc="gcc-7" cxx="g++-7"'
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
export CFLAGS='
	-fopenmp -Ofast -ftree-vectorize
	-fopt-info-vec-optimized -funsafe-math-optimizations
	-funsafe-loop-optimizations -fPIC'
```
Configure libmypaint:
```
./configure --enable-static --enable-openmp --enable-shared=false
```
Build libmypaint:
```
sudo make
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
sudo apt-get install libswresample-dev
	libswscale-dev libavcodec-dev
	libavformat-dev libavresample-dev
```

#### Other
enve dependencies:
```
sudo apt-get install libgoogle-perftools-dev
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
make
cd ..
```
Build a debug version of enve (if needed):
```
cd Debug
qmake CONFIG+=debug ../../enve.pro
make
cd ..
```
Now you have successuly build enve and libenvecore along with all the examples.
If you wish to create your own executable proceed to the **Deployment** section.

## Deployment

Make sure you are in the build directory.

### Download needed tools

Download, rename, and change permission for AppImageKit:
```
wget https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage
mv appimagetool-x86_64.AppImage appimagetool.AppImage
chmod +x appimagetool.AppImage
```
Download, rename, and change permission for LinuxDeployQt:
```
wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
mv linuxdeployqt-6-x86_64.AppImage linuxdeployqt.AppImage
chmod +x linuxdeployqt.AppImage
```
Copy the enve build into the AppDir:
```
cp Release/src/app/enve AppDir/usr/bin/
cp -av Release/src/core/*.so* AppDir/usr/lib/
```
Copy all necessary dependencies into the AppDir by using LinuxDeployQt:
```
./linuxdeployqt.AppImage AppDir/usr/share/applications/enve.desktop -appimage
```

#### Support older systems (ex. Ubuntu 16.04)
Download patched AppRun binary and an associated library that will link a newer version of libstdc++ on older systems:
```
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64
mv AppRun-patched-x86_64 AppRun
chmod +x AppRun
```
```
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so
mv exec-x86_64.so exec.so
```
Copy everything required to use newer libstdc++ on older systems to your AppDir:
```
mkdir AppDir/usr/optional/libstdc++
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/optional/libstdc++/
cp exec.so AppDir/usr/optional
```

#### Create AppImage

Create the AppImage using AppImageTool:
```
cp AppRun AppDir/
./appimagetool.AppImage AppDir
```

You have successfuly created your own enve AppImage!

## Authors

**Maurycy Liebner** - 2016-2019 - [MaurycyLiebner](https://github.com/MaurycyLiebner)

## License

This project is licensed under the GPL3 License - see the [LICENSE.md](LICENSE.md) file for details
