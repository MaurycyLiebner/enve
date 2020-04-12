# Build third party dependencies
.PHONY: all patch build skia libmypaint quazip gperftools qscintilla

all: patch build

patch:
	patch -d gperftools -p1 < gperftools-enve-mod.patch

# install_deps:
# TODO: Integrate third-party build commands to Makefile,
# making it easier for CI and contributors to start up.

build: skia libmypaint quazip gperftools qscintilla

.ONESHELL:
skia:
	cd skia
	python tools/git-sync-deps
	bin/gn gen out/Release --args='is_official_build=true is_debug=false extra_cflags=["-Wno-error"] skia_use_system_expat=false skia_use_system_icu=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false'
	ninja -C out/Release skia

.ONESHELL:
libmypaint:
	cd libmypaint
	./autogen.sh
	./configure --enable-static --enable-shared=false
	make
	ln -s `pwd` libmypaint

.ONESHELL:
quazip:
	cd quazip
	qmake quazip.pro CONFIG+=release
	make

.ONESHELL:
gperftools:
	cd gperftools
	./autogen.sh
	./configure --prefix /usr LIBS=-lpthread
	make

.ONESHELL:
qscintilla:
	cd qscintilla
	qmake CONFIG+=release
	make
