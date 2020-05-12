ENVE_VERSION = 0.0.0c

DEFINES += ENVE_VERSION=\\\"$$ENVE_VERSION\\\"

exists(../../.git/index) {
    LATEST_COMMIT_HASH = $$system(git log --pretty=format:'%h' -n 1)
    LATEST_COMMIT_DATE = $$system(git log -1 --pretty=%cd --date=short)

    DEFINES += LATEST_COMMIT_HASH=\\\"$$LATEST_COMMIT_HASH\\\"
    DEFINES += LATEST_COMMIT_DATE=\\\"$$LATEST_COMMIT_DATE\\\"
}

# Header for third-party dependencies

ENVE_FOLDER = $$_PRO_FILE_PWD_/../..
THIRD_PARTY_FOLDER = $$ENVE_FOLDER/third_party
SKIA_FOLDER = $$THIRD_PARTY_FOLDER/skia
LIBMYPAINT_FOLDER = $$THIRD_PARTY_FOLDER/libmypaint
QUAZIP_FOLDER = $$THIRD_PARTY_FOLDER/quazip

INCLUDEPATH += $$SKIA_FOLDER
QMAKE_CFLAGS += -isystem $$SKIA_FOLDER

INCLUDEPATH += $$LIBMYPAINT_FOLDER
LIBS += -L$$LIBMYPAINT_FOLDER/.libs -lmypaint

INCLUDEPATH += $$QUAZIP_FOLDER
LIBS += -L$$QUAZIP_FOLDER/quazip -lquazip

CONFIG(debug, debug|release) {
    LIBS += -L$$SKIA_FOLDER/out/Debug
} else {
    LIBS += -L$$SKIA_FOLDER/out/Release
}
LIBS += -lskia

win32 { # Windows
    FFMPEG_FOLDER = $$THIRD_PARTY_FOLDER/ffmpeg-4.2.2-win64-dev
    LIBS += -L$$FFMPEG_FOLDER/lib
    INCLUDEPATH += $$FFMPEG_FOLDER/include

    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib

    QMAKE_CFLAGS_RELEASE += /O2 -O2 /GL
    QMAKE_CXXFLAGS_RELEASE += /O2 -O2 /GL

    QMAKE_CFLAGS += -openmp
    QMAKE_CXXFLAGS += -openmp

    LIBS += -L"C:\Program Files\LLVM\lib" -llibomp
    LIBS += -luser32 -lopengl32

    CONFIG -= debug_and_release
} unix {
    macx { # Mac OS X
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13
        LIBS += -framework CoreFoundation
        LIBS += -framework CoreGraphics
        LIBS += -framework CoreText
        LIBS += -framework CoreServices
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
        LIBS += -lpthread -lfontconfig -lfreetype -lpng -ldl -fopenmp
    }
}
