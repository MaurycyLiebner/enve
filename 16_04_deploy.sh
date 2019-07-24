#!/bin/sh -e
cd build/Release
qmake ../../enve.pro
make
cd ../
cp Release/src/app/enve AppDir/usr/bin/
cp -av Release/src/core/*.so* AppDir/usr/lib/
./linuxdeployqt.AppImage AppDir/usr/share/applications/enve.desktop -appimage
mkdir AppDir/usr/optional/libstdc++
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/optional/libstdc++/
cp exec.so AppDir/usr/optional
cp AppRun AppDir/
./appimagetool.AppImage AppDir