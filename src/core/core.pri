# Header for third-party dependencies

ENVE_FOLDER = $$_PRO_FILE_PWD_/../..
THIRD_PARTY_FOLDER = $$ENVE_FOLDER/third_party
SKIA_FOLDER = $$THIRD_PARTY_FOLDER/skia
LIBMYPAINT_FOLDER = $$THIRD_PARTY_FOLDER/libmypaint
QUAZIP_FOLDER = $$THIRD_PARTY_FOLDER/quazip

INCLUDEPATH += $$SKIA_FOLDER

INCLUDEPATH += $$LIBMYPAINT_FOLDER
LIBS += -L$$LIBMYPAINT_FOLDER/.libs

INCLUDEPATH += $$QUAZIP_FOLDER
LIBS += -L$$QUAZIP_FOLDER/quazip -lquazip

CONFIG(debug, debug|release) {
    LIBS += -L$$SKIA_FOLDER/out/Debug
} else {
    LIBS += -L$$SKIA_FOLDER/out/Release
}

LIBS += -lskia -lmypaint

win32 { # Windows
    FFMPEG_FOLDER = $$THIRD_PARTY_FOLDER/ffmpeg-4.2.2-win64-dev
    LIBS += -L$$FFMPEG_FOLDER/lib
    INCLUDEPATH += $$FFMPEG_FOLDER/include

    INCLUDEPATH += $$THIRD_PARTY_FOLDER/zlib/src
    LIBS += -L$$THIRD_PARTY_FOLDER/zlib -lzlib

    QMAKE_CFLAGS_RELEASE += /O2 -O2 /GL
    QMAKE_LFLAGS_RELEASE += /LTCG
    QMAKE_CXXFLAGS_RELEASE += /O2 -O2 /GL

    QMAKE_CFLAGS += -openmp
    QMAKE_CXXFLAGS += -openmp

    CONFIG -= debug_and_release
} unix {
    macx { # Mac OS X
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
        INCLUDEPATH += /usr/local/include
        LIBS += -L/usr/local/lib
    } else { # Linux
        LIBS += -lgobject-2.0 -lglib-2.0 -ljson-c

        QMAKE_CFLAGS_RELEASE -= -O2
        QMAKE_CFLAGS_RELEASE -= -O1
        QMAKE_CXXFLAGS_RELEASE -= -O2
        QMAKE_CXXFLAGS_RELEASE -= -O1
        QMAKE_CFLAGS_RELEASE += -m64 -O3
        QMAKE_CXXFLAGS_RELEASE += -m64 -O3

        QMAKE_CXXFLAGS += -fopenmp
        LIBS += -lfreetype -lpng -ldl -fopenmp# -lX11
    }
}
