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

#include <fstream>
#include "Animators/qrealanimator.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
#include "Animators/qstringanimator.h"
#include "Animators/transformanimator.h"
#include "Animators/paintsettingsanimator.h"
#include "Animators/qrealanimator.h"
#include "Animators/gradient.h"
#include "Properties/comboboxproperty.h"
#include "PathEffects/patheffectcollection.h"
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
#include "Boxes/internallinkbox.h"
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
#include "ReadWrite/basicreadwrite.h"
#include "Boxes/internallinkcanvas.h"
#include "Boxes/smartvectorpath.h"
#include "Sound/soundcomposition.h"
#include "RasterEffects/rastereffectcollection.h"
#include "ReadWrite/filefooter.h"
#include "GUI/timelinedockwidget.h"
#include "GUI/RenderWidgets/renderwidget.h"

void MainWindow::loadEVFile(const QString &path) {
    QFile file(path);
    if(!file.exists()) RuntimeThrow("File does not exist " + path);
    if(!file.open(QIODevice::ReadOnly))
        RuntimeThrow("Could not open file " + path);
    try {
        const int evVersion = FileFooter::sReadEvFileVersion(&file);
        if(evVersion <= 0) RuntimeThrow("Incompatible or incomplete data");
        eReadStream readStream(evVersion, &file);

        const qint64 savedPos = file.pos();
        const qint64 pos = file.size() - FileFooter::sSize(evVersion) -
                qint64(sizeof(int));
        file.seek(pos);
        readStream.readFutureTable();
        file.seek(savedPos);
        readStream.readCheckpoint("File beginning pos mismatch");
        mDocument.read(readStream);
        readStream.readCheckpoint("Error reading Document");
        mLayoutHandler->read(readStream);
        readStream.readCheckpoint("Error reading Layout");
        if(readStream.evFileVersion() > 4) {
            const auto renderWidget = mTimeline->getRenderWidget();
            renderWidget->read(readStream);
            readStream.readCheckpoint("Error reading Render Widget");
        }
    } catch(...) {
        file.close();
        RuntimeThrow("Error while reading from file " + path);
    }
    file.close();
    addRecentFile(path);

    BoundingBox::sClearReadBoxes();
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
        const auto renderWidget = mTimeline->getRenderWidget();
        renderWidget->write(writeStream);
        writeStream.writeCheckpoint();

        writeStream.writeFutureTable();
        FileFooter::sWrite(writeStream);
    } catch(...) {
        file.close();
        RuntimeThrow("Error while writing to file " + path);
    }
    file.close();

    BoundingBox::sClearWriteBoxes();
    addRecentFile(path);
}

#include "zipfilesaver.h"

void MainWindow::saveToFileXEV(const QString &path) {
    QFile file(path);
    if(file.exists()) file.remove();

    if(!file.open(QIODevice::WriteOnly))
        RuntimeThrow("Could not open file for writing " + path + ".");
    try {
        ZipFileSaver fileSaver;
        fileSaver.setIoDevice(&file);
        fileSaver.processText("mimetype", [](QTextStream& stream) {
            stream << "application/enve";
        }, false);

        fileSaver.process("Thumbnails/thumbnail.png", [this](QIODevice* const dst) {
            const qreal scale = 256./width();
            QImage img(qRound(width()*scale),
                       qRound(height()*scale),
                       QImage::Format_RGB888);
            QPainter p(&img);
            p.scale(scale, scale);
            render(&p);
            img.save(dst, "PNG");
        });

        mDocument.writeXEV(fileSaver);
    } catch(...) {
        file.close();
        RuntimeThrow("Error while writing to file " + path);
    }
    file.close();

    BoundingBox::sClearWriteBoxes();
    addRecentFile(path);
}


void MainWindow::loadXevFile(const QString &path) {
    try {
        ZipFileLoader fileLoader;
        fileLoader.setZipPath(path);

        mDocument.readXEV(fileLoader);
    } catch(...) {
        RuntimeThrow("Error while reading from file " + path);
    }

    BoundingBox::sClearReadBoxes();
    addRecentFile(path);
}
