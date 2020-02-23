// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "GUI/mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QProcess>
#include "hardwareinfo.h"
#include "Private/esettings.h"
#include "GUI/ewidgetsimpl.h"
#include "importhandler.h"
#include "effectsloader.h"
#include "memoryhandler.h"
#include "ShaderEffects/shadereffectprogram.h"
#include "videoencoder.h"
#include "iconloader.h"
#include "GUI/envesplash.h"
extern "C" {
    #include <libavformat/avformat.h>
}

int FONT_HEIGHT;
int MIN_WIDGET_DIM;
int BUTTON_DIM;
int KEY_RECT_SIZE;

QPixmap* ALPHA_MESH_PIX;

void setDefaultFormat() {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(0);
    //format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    //format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[]) {
    setDefaultFormat();
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    FONT_HEIGHT = QApplication::fontMetrics().height();
    MIN_WIDGET_DIM = FONT_HEIGHT*4/3;
    BUTTON_DIM = qRound(MIN_WIDGET_DIM*1.1);
    KEY_RECT_SIZE = MIN_WIDGET_DIM*3/5;
    QPixmap alphaMeshPix;
    {
        const int dim = MIN_WIDGET_DIM/2;
        alphaMeshPix = QPixmap(2*dim, 2*dim);
        ALPHA_MESH_PIX = &alphaMeshPix;
        const QColor light = QColor::fromRgbF(0.2, 0.2, 0.2);
        const QColor dark = QColor::fromRgbF(0.4, 0.4, 0.4);
        QPainter p(ALPHA_MESH_PIX);
        p.fillRect(0, 0, dim, dim, light);
        p.fillRect(dim, 0, dim, dim, dark);
        p.fillRect(0, dim, dim, dim, dark);
        p.fillRect(dim, dim, dim, dim, light);
        p.end();
    }

    //#ifdef QT_DEBUG
    //    const qint64 pId = QCoreApplication::applicationPid();
    //    QProcess * const process = new QProcess(&w);
    //    process->start("prlimit --data=3000000000 --pid " + QString::number(pId));
    //#endif

    HardwareInfo::sUpdateInfo();
    eSettings settings(HardwareInfo::sCpuThreads(),
                       HardwareInfo::sRamKB(),
                       HardwareInfo::sGpuVendor());
    eFilterSettings filterSettings;
    QDir(eSettings::sSettingsDir()).mkpath(eSettings::sIconsDir());
    try {
        const QString pngPath = eSettings::sIconsDir() + "/splash1.png";
#ifdef QT_DEBUG
        QFile(pngPath).remove();
#endif
        IconLoader::generate(":/pixmaps/splash.svg", MIN_WIDGET_DIM/22., pngPath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    const auto splash = new EnveSplash;
    splash->show();
    app.processEvents();

    splash->showMessage("Load settings...");
    app.processEvents();
    try {
        settings.loadFromFile();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }

    splash->showMessage("Generate icons...");
    app.processEvents();
    IconLoader::generateAll(MIN_WIDGET_DIM, BUTTON_DIM);

    eWidgetsImpl widImpl;
    ImportHandler importHandler;

    splash->showMessage("Initialize task scheduler...");
    app.processEvents();
    MemoryHandler memoryHandler;
    TaskScheduler taskScheduler;
    QObject::connect(&memoryHandler, &MemoryHandler::enteredCriticalState,
                     &taskScheduler, &TaskScheduler::enterCriticalMemoryState);
    QObject::connect(&memoryHandler, &MemoryHandler::finishedCriticalState,
                     &taskScheduler, &TaskScheduler::finishCriticalMemoryState);
    try {
        taskScheduler.initializeGpu();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
    }
    Document document(taskScheduler);
    Actions actions(document);

    splash->showMessage("Initialize gpu resources...");
    app.processEvents();
    EffectsLoader effectsLoader;
    effectsLoader.initializeGpu();

    splash->showMessage("Initialize custom path effects...");
    app.processEvents();
    effectsLoader.iniCustomPathEffects();

    splash->showMessage("Initialize custom raster effects...");
    app.processEvents();
    effectsLoader.iniCustomRasterEffects();

    splash->showMessage("Initialize shader effects...");
    app.processEvents();
    effectsLoader.iniShaderEffects();
    QObject::connect(&effectsLoader, &EffectsLoader::programChanged,
    [&document](ShaderEffectProgram * program) {
        for(const auto& scene : document.fScenes)
            scene->updateIfUsesProgram(program);
        document.actionFinished();
    });

    splash->showMessage("Initialize custom objects...");
    app.processEvents();
    effectsLoader.iniCustomBoxes();


    splash->showMessage("Initialize audio...");
    app.processEvents();

    eSoundSettings soundSettings;
    AudioHandler audioHandler;
    audioHandler.initializeAudio(soundSettings.sData());

    splash->showMessage("Initialize render handler...");
    app.processEvents();
    const auto videoEncoder = enve::make_shared<VideoEncoder>();
    RenderHandler renderHandler(document, audioHandler,
                                *videoEncoder, memoryHandler);

    MainWindow w(document, actions, audioHandler, renderHandler);
    if(argc > 1) {
        try {
            splash->showMessage("Load file...");
            app.processEvents();
            w.loadEVFile(argv[1]);
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
    }
    splash->showMessage("Done");
    app.processEvents();
    w.show();

    const bool keepSplashVisible = true;
    if(keepSplashVisible) {
        splash->setParent(&w);
        splash->move(splash->pos() + w.mapFromGlobal({0, 0}));
        splash->show();
    } else splash->finish(&w);

    try {
        return app.exec();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
        return -1;
    }
}
