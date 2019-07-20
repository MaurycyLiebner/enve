#include <fstream>
#include "Animators/qrealanimator.h"
#include "Animators/randomqrealgenerator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
#include "Animators/effectanimators.h"
#include "PixmapEffects/pixmapeffect.h"
#include "Animators/qstringanimator.h"
#include "Animators/transformanimator.h"
#include "Animators/paintsettingsanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/gradient.h"
#include "Properties/comboboxproperty.h"
#include "Properties/intproperty.h"
#include "PathEffects/patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/boundingbox.h"
#include "Boxes/pathbox.h"
#include "Boxes/containerbox.h"
#include "Boxes/rectangle.h"
#include "Boxes/circle.h"
#include "Boxes/imagebox.h"
#include "Boxes/videobox.h"
#include "Boxes/textbox.h"
#include "Boxes/particlebox.h"
#include "Boxes/imagesequencebox.h"
#include "Boxes/linkbox.h"
#include "Boxes/paintbox.h"
//#include "GUI/BrushWidgets/brushsettingswidget.h"
#include "canvas.h"
#include "GUI/Timeline/durationrectangle.h"
#include "Animators/gradientpoints.h"
#include "MovablePoints/gradientpoint.h"
#include "Animators/qrealkey.h"
#include "GUI/mainwindow.h"
#include "GUI/canvaswindow.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include <QMessageBox>
#include "PathEffects/patheffectsinclude.h"
#include "PixmapEffects/pixmapeffectsinclude.h"
#include "basicreadwrite.h"
#include "Boxes/internallinkcanvas.h"
#include "Boxes/smartvectorpath.h"
#include "Sound/singlesound.h"
#include "Sound/soundcomposition.h"
#include "Animators/gpueffectanimators.h"


void FixedLenAnimationRect::writeDurationRectangle(QIODevice *target) {
    DurationRectangle::writeDurationRectangle(target);
    target->write(rcConstChar(&mBoundToAnimation), sizeof(bool));
    target->write(rcConstChar(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->write(rcConstChar(&mMinAnimationFrame), sizeof(int));
    target->write(rcConstChar(&mMaxAnimationFrame), sizeof(int));
}

void FixedLenAnimationRect::readDurationRectangle(QIODevice *target) {
    DurationRectangle::readDurationRectangle(target);
    int minFrame;
    int maxFrame;
    target->read(rcChar(&mBoundToAnimation), sizeof(bool));
    target->read(rcChar(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->read(rcChar(&minFrame), sizeof(int));
    target->read(rcChar(&maxFrame), sizeof(int));
    setMinAnimationFrame(minFrame);
    setMaxAnimationFrame(maxFrame);
}

class FileFooter {
public:
    static bool sWrite(QIODevice * const target) {
        return target->write(rcConstChar(sEVFormat), sizeof(char[15])) &&
               target->write(rcConstChar(sAppName), sizeof(char[15])) &&
               target->write(rcConstChar(sAppVersion), sizeof(char[15]));
    }

    static bool sCompatible(QIODevice *target) {
        const qint64 savedPos = target->pos();
        const qint64 pos = target->size() -
                static_cast<qint64>(3*sizeof(char[15]));
        if(!target->seek(pos)) RuntimeThrow("Failed to seek to FileFooter");

        char format[15];
        target->read(rcChar(format), sizeof(char[15]));
        if(std::strcmp(format, sEVFormat)) return false;

//        char appVersion[15];
//        target->read(rcChar(appVersion), sizeof(char[15]));

//        char appName[15];
//        target->read(rcChar(appName), sizeof(char[15]));

        if(!target->seek(savedPos))
            RuntimeThrow("Could not restore current position for QIODevice.");
        return true;
    }
private:
    static char sEVFormat[15];
    static char sAppName[15];
    static char sAppVersion[15];
};

char FileFooter::sEVFormat[15] = "enve ev";
char FileFooter::sAppName[15] = "enve";
char FileFooter::sAppVersion[15] = "0.5";

void MainWindow::loadEVFile(const QString &path) {
    QFile file(path);
    if(!file.exists()) RuntimeThrow("File does not exist " + path);
    if(!file.open(QIODevice::ReadOnly))
        RuntimeThrow("Could not open file " + path);

    try {
        if(!FileFooter::sCompatible(&file)) {
            RuntimeThrow("Incompatible or incomplete data");
        }
        mDocument.read(&file);
        mLayoutHandler->read(&file);
    } catch(...) {
        file.close();
        RuntimeThrow("Error while reading from file " + path);
    }
    file.close();

    BoundingBox::sClearReadBoxes();
    addRecentFile(path);
}

void MainWindow::saveToFile(const QString &path) {
    QFile file(path);
    if(file.exists()) file.remove();

    if(!file.open(QIODevice::WriteOnly))
        RuntimeThrow("Could not open file for writing " + path + ".");

    try {
        mDocument.write(&file);
        mLayoutHandler->write(&file);

        FileFooter::sWrite(&file);
    } catch(...) {
        file.close();
        RuntimeThrow("Error while writing to file " + path);
    }
    file.close();

    BoundingBox::sClearWriteBoxes();
    addRecentFile(path);
}
