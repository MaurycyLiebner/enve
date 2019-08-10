#include "GUI/mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QProcess>
#include "hardwareinfo.h"
#include "GUI/ewidgetsimpl.h"
#include "importhandler.h"
#include "effectsloader.h"
#include "memoryhandler.h"
#include "ShaderEffects/shadereffectprogram.h"
#include "videoencoder.h"
#include "iconloader.h"
extern "C" {
    #include <libavformat/avformat.h>
}

int FONT_HEIGHT;
int MIN_WIDGET_DIM;
int BUTTON_DIM;
int KEY_RECT_SIZE;

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
    QApplication a(argc, argv);
    //#ifdef QT_DEBUG
    //    const qint64 pId = QCoreApplication::applicationPid();
    //    QProcess * const process = new QProcess(&w);
    //    process->start("prlimit --data=3000000000 --pid " + QString::number(pId));
    //#endif

    HardwareInfo::sUpdateInfo();

    eWidgetsImpl widImpl;
    ImportHandler importHandler;
    TaskScheduler taskScheduler;
    Document document(taskScheduler);
    Actions actions(document);

    EffectsLoader effectsLoader;
    effectsLoader.initialize();
    QObject::connect(&effectsLoader, &EffectsLoader::programChanged,
    [&document](ShaderEffectProgram * program) {
        for(const auto& scene : document.fScenes)
            scene->updateIfUsesProgram(program);
        document.actionFinished();
    });

    av_register_all();
    AudioHandler audioHandler;
    audioHandler.initializeAudio();

    MemoryHandler memoryHandler;

    const auto videoEncoder = enve::make_shared<VideoEncoder>();
    RenderHandler renderHandler(document, audioHandler,
                                *videoEncoder, memoryHandler);

    FONT_HEIGHT = QApplication::fontMetrics().height();
    MIN_WIDGET_DIM = FONT_HEIGHT*4/3;
    BUTTON_DIM = qRound(MIN_WIDGET_DIM*1.1);
    KEY_RECT_SIZE = MIN_WIDGET_DIM*3/5;

    IconLoader::generateAll(MIN_WIDGET_DIM, BUTTON_DIM);

    MainWindow w(document, actions, audioHandler, renderHandler);
    w.show();
    if(argc > 1) {
        try {
            w.loadEVFile(argv[1]);
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
    }

    try {
        return a.exec();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
        return -1;
    }
}
