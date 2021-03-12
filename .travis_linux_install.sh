wget https://raw.githubusercontent.com/qbs/qbs/master/scripts/install-qt.sh
chmod +x install-qt.sh
./install-qt.sh -d ${QT_INSTALL_DIR} --version ${QT_VERSION} qtbase qttools qtsvg qtmultimedia qtdeclarative qtwebengine qtwebchannel qtwebview qtlocation qtserialport icu

# Travis clones submodules for us, only library patches are needed
cd third_party
make patch

# build skia
if ! [ -d $TRAVIS_CACHE_DIR/third_party/skia/out/Release ]; then
    cd skia
    python tools/git-sync-deps
    # sudo tools/install_dependencies.sh
    wget https://github.com/ninja-build/ninja/releases/download/v1.9.0/ninja-linux.zip
    unzip ninja-linux.zip
    chmod +x ninja
    bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] target_os="linux" target_cpu="x64" skia_use_system_expat=false skia_use_system_freetype2=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_icu=false skia_use_system_harfbuzz=false cc="clang" cxx="clang++"'
    ./ninja -C out/Release -j 2 skia
    mkdir -p $TRAVIS_CACHE_DIR/third_party/skia/out/Release
    cp -r ./out/Release/* $TRAVIS_CACHE_DIR/third_party/skia/out/Release
    cd ..
else
    mkdir -p ./skia/out/Release
    cp -r $TRAVIS_CACHE_DIR/third_party/skia/out/Release/* ./skia/out/Release
fi

# build libmypaint
cd libmypaint
export CFLAGS='-fopenmp -Ofast -ftree-vectorize -fopt-info-vec-optimized -funsafe-math-optimizations -funsafe-loop-optimizations -fPIC'
./autogen.sh
./configure --enable-static --enable-openmp --enable-shared=false
sudo make -j 2
ln -s `pwd` libmypaint
unset CFLAGS
cd ..

# build quazip
cd quazip/quazip
qmake
make -j 2
sudo make install
cd ../../

# build gperftools
cd gperftools
./autogen.sh
./configure --prefix /usr LIBS=-lpthread
make -j 2
cd ..

# build QScintilla
cd qscintilla/Qt4Qt5
qmake
make -j 2
sudo make install
cd ../../

# build qtstyleplugins
git clone https://github.com/qt/qtstyleplugins
cd qtstyleplugins
qmake
make -j 2
make install
cd ../../