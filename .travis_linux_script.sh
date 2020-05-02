cd build
wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
chmod +x linuxdeployqt-6-x86_64.AppImage
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64
mv AppRun-patched-x86_64 AppRun
chmod +x AppRun
wget https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so
mv exec-x86_64.so exec.so
cd Release

# embed commit hash and date
LATEST_COMMIT_HASH=\"$(git log --pretty=format:'%h' -n 1)\"
enveSplash=../../src/app/GUI/envesplash.h
echo "#ifndef LATEST_COMMIT_HASH" >> $enveSplash
echo "#define LATEST_COMMIT_HASH $LATEST_COMMIT_HASH" >> $enveSplash
echo "#endif" >> $enveSplash
LATEST_COMMIT_DATE=\"$(git log -1 --format=%ai)\"
echo "#ifndef LATEST_COMMIT_DATE" >> $enveSplash
echo "#define LATEST_COMMIT_DATE $LATEST_COMMIT_DATE" >> $enveSplash
echo "#endif" >> $enveSplash
qmake CONFIG+=build_examples ../../enve.pro
make -j 2 CC=gcc-7 CPP=g++-7 CXX=g++-7 LD=g++-7
cd ..

# Generate AppImage
cp AppRun AppDir/
cp exec.so AppDir/usr/optional
mkdir AppDir/usr/optional/libstdc++
cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 AppDir/usr/optional/libstdc++/
cp Release/src/app/enve AppDir/usr/bin/
cp -av Release/src/core/*.so* AppDir/usr/lib/
cp -av ../third_party/gperftools/.libs/libtcmalloc.so* AppDir/usr/lib/
./linuxdeployqt-6-x86_64.AppImage AppDir/usr/share/applications/enve.desktop -appimage