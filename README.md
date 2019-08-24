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
Enter skia directory:
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
Build the release version of skia (you can use more than 2 threads):
```
./ninja -C out/Release -j 2
```
If you want, you can also build debug version:
```
bin/gn gen out/Debug --args='extra_cflags=["-Wno-error"]'
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

#### ffmpeg
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
Otherwise it will not execute.
```
sudo apt-get install libxkbcommon-x11-dev
```

#### Qt

Download Qt 5.12.4 installer:
```
wget http://download.qt.io/official_releases/qt/5.12/5.12.4/qt-opensource-linux-x64-5.12.4.run
```
Set the permisson for the installer:
```
chmod +x qt-opensource-linux-x64-5.12.4.run
```
Run the installer:
```
./qt-opensource-linux-x64-5.12.4.run
```

Install Qt in the directory of your choice (ex. ~/.Qt)

### enve itself

You can either build enve through QtCreator (open enve.pro),
or by running qmake and make directly.
For release version:
```
cd build/Release
qmake ../../enve.pro
make
cd ..
```
For debug version (if needed):
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

Download AppImageKit:
```
wget https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage
mv appimagetool-x86_64.AppImage appimagetool.AppImage
chmod +x appimagetool.AppImage
```
Download LinuxDeployQt:
```
wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
mv linuxdeployqt-6-x86_64.AppImage linuxdeployqt.AppImage
chmod +x linuxdeployqt.AppImage
```
Download patched AppRun binary to check libstdc++ dependencies at runtime:
```
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so
mv exec-x86_64.so exec.so
```
```
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64
mv AppRun-patched-x86_64 AppRun
chmod +x AppRun
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
Copy libraries required to check and use libstdc++ dependencies at runtime:
```
mkdir AppDir/usr/optional/libstdc++
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/optional/libstdc++/
cp exec.so AppDir/usr/optional
```
Create the AppImage using AppImageTool:
```
cp AppRun AppDir/
./appimagetool.AppImage AppDir
```

You have successfuly created your own enve AppImage!

## Authors

**Maurycy Liebner** - [MaurycyLiebner](https://github.com/MaurycyLiebner)

## License

This project is licensed under the GPL3 License - see the [LICENSE.md](LICENSE.md) file for details
