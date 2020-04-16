brew link qt5 --force
brew link intltool --force

# Travis clones submodules for us, only library patches are needed
cd third_party
make patch

# build skia
if ! [ -d $TRAVIS_CACHE_DIR/third_party/skia/out/Release ]; then
    cd skia
    python2 tools/git-sync-deps
    wget https://github.com/ninja-build/ninja/releases/download/v1.9.0/ninja-mac.zip
    unzip ninja-mac.zip
    chmod +x ninja
    bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] skia_use_system_expat=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false'
    ./ninja -C out/Release -j 2
    mkdir -p $TRAVIS_CACHE_DIR/third_party/skia/out/Release
    cp -r ./out/Release/* $TRAVIS_CACHE_DIR/third_party/skia/out/Release
    cd ..
else
    mkdir -p ./skia/out/Release
    cp -r $TRAVIS_CACHE_DIR/third_party/skia/out/Release/* ./skia/out/Release
fi

# build libmypaint
cd libmypaint
ACLOCAL_FLAGS="-I/usr/local/opt/gettext/share/aclocal $ACLOCAL_FLAGS"
LDFLAGS="-L/usr/local/opt/gettext/lib $LDFLAGS"
CPPFLAGS="-I/usr/local/opt/gettext/include $CPPFLAGS"
PATH="/usr/local/opt/gettext/bin:$PATH"
./autogen.sh
./configure --enable-openmp --prefix=/usr/local
make
ln -s `pwd` libmypaint
cd ..

# build quazip
cd quazip/quazip
qmake quazip.pro -spec macx-clang CONFIG+=release CONFIG+=x86_64 INCLUDEPATH+=$$[QT_INSTALL_HEADERS]/QtZlib
make -j 2
sudo make install
cd ../../

# build gperftools
cd gperftools
CFLAGS="$CFLAGS -Wno-error -D_XOPEN_SOURCE"
./autogen.sh
./configure --disable-dependency-tracking --prefix=/usr/local
make -j 2
cd ..

# build QScintilla
cd qscintilla/Qt4Qt5
qmake -spec macx-clang CONFIG+=release
make -j 2
sudo make install
cd ../../../