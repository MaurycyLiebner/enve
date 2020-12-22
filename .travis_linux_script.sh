INSTALL_PREFIX=$PWD/build/AppDir/usr

cd build
wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
chmod +x linuxdeployqt-6-x86_64.AppImage
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64
mv AppRun-patched-x86_64 AppRun
chmod +x AppRun
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so
mv exec-x86_64.so exec.so
cd Release
qmake PREFIX=$INSTALL_PREFIX CONFIG+=build_examples ../../enve.pro
make -j 2 CC=gcc-7 CPP=g++-7 CXX=g++-7 LD=g++-7
make install
cd ..

# Generate AppImage
cp AppRun AppDir/
cp exec.so AppDir/usr/optional
mkdir AppDir/usr/optional/libstdc++
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/optional/libstdc++/
cp -av ../third_party/gperftools/.libs/libtcmalloc.so* AppDir/usr/lib/
./linuxdeployqt-6-x86_64.AppImage AppDir/usr/share/applications/io.github.maurycyliebner.enve.desktop -appimage -extra-plugins=platformthemes/libqgtk2.so,styles/libqgtk2style.so
