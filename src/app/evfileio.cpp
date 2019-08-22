#include <fstream>
#include "Animators/qrealanimator.h"
#include "Animators/randomqrealgenerator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
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
#include "Boxes/imagesequencebox.h"
#include "Boxes/linkbox.h"
#include "Boxes/paintbox.h"
//#include "GUI/BrushWidgets/brushsettingswidget.h"
#include "canvas.h"
#include "Timeline/durationrectangle.h"
#include "Animators/gradientpoints.h"
#include "MovablePoints/gradientpoint.h"
#include "Animators/qrealkey.h"
#include "GUI/mainwindow.h"
#include "GUI/canvaswindow.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include <QMessageBox>
#include "PathEffects/patheffectsinclude.h"
#include "basicreadwrite.h"
#include "Boxes/internallinkcanvas.h"
#include "Boxes/smartvectorpath.h"
#include "Sound/singlesound.h"
#include "Sound/soundcomposition.h"
#include "Animators/rastereffectanimators.h"

void MainWindow::loadEVFile(const QString &path) {
    QFile file(path);
    if(!file.exists()) RuntimeThrow("File does not exist " + path);
    if(!file.open(QIODevice::ReadOnly))
        RuntimeThrow("Could not open file " + path);
    eReadStream readStream(&file);

    try {
        if(!FileFooter::sCompatible(&file))
            RuntimeThrow("Incompatible or incomplete data");
        readStream.readCheckpoint("File beginning pos mismatch");
        mDocument.read(readStream);
        readStream.readCheckpoint("Error reading Document");
        mLayoutHandler->read(readStream);
        readStream.readCheckpoint("Error reading Layout");
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
    eWriteStream writeStream(&file);

    try {
        writeStream.writeCheckpoint();
        mDocument.write(writeStream);
        writeStream.writeCheckpoint();
        mLayoutHandler->write(writeStream);
        writeStream.writeCheckpoint();

        const qint64 tableSize = file.write(writeStream.futureData());
        file.write(rcConstChar(&tableSize), sizeof(qint64));
        FileFooter::sWrite(&file);
    } catch(...) {
        file.close();
        RuntimeThrow("Error while writing to file " + path);
    }
    file.close();

    BoundingBox::sClearWriteBoxes();
    addRecentFile(path);
}
