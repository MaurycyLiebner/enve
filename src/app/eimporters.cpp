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

#include "eimporters.h"

#include "GUI/mainwindow.h"
#include "eimporters.h"
#include "svgimporter.h"
#include "Ora/oraimporter.h"

qsptr<BoundingBox> eXevImporter::import(const QFileInfo &fileInfo, Canvas * const scene) const {
    Q_UNUSED(scene);
    MainWindow::sGetInstance()->loadXevFile(fileInfo.absoluteFilePath());
    return nullptr;
}

qsptr<BoundingBox> evImporter::import(const QFileInfo &fileInfo, Canvas * const scene) const {
    Q_UNUSED(scene);
    MainWindow::sGetInstance()->loadEVFile(fileInfo.absoluteFilePath());
    return nullptr;
}

qsptr<BoundingBox> eSvgImporter::import(const QFileInfo &fileInfo, Canvas * const scene) const {
    const auto gradientCreator = [scene]() {
        return scene->createNewGradient();
    };
    return ImportSVG::loadSVGFile(fileInfo.absoluteFilePath(),
                                  gradientCreator);
}

qsptr<BoundingBox> eOraImporter::import(const QFileInfo &fileInfo, Canvas * const scene) const {
    const auto gradientCreator = [scene]() {
        return scene->createNewGradient();
    };
    return ImportORA::loadORAFile(fileInfo.absoluteFilePath(),
                                  gradientCreator);
}

qsptr<BoundingBox> eKraImporter::import(const QFileInfo &fileInfo, Canvas * const scene) const {
    QProcess oraConv;
    oraConv.setProcessChannelMode(QProcess::ForwardedChannels);
    const QString kritaPath = eSettings::sInstance->fKrita;
    const QString oraPath = eSettings::sSettingsDir() + "/e_kra_to_ora.ora";
    if(kritaPath.isEmpty()) RuntimeThrow("Invalid path to Krita executable.");
    const QString command = QString("%1 %2 --export --export-filename %3").
            arg(kritaPath).arg(fileInfo.absoluteFilePath()).arg(oraPath);
    oraConv.start(command);
    const bool success = oraConv.waitForFinished(10000);
    if(!success) gPrintException(false, "Could not use Krita to convert the kra file to ora.\n"
                                        "Please make sure you defined a proper path to Krita executable.");
    const auto gradientCreator = [scene]() {
        return scene->createNewGradient();
    };
    const auto result = ImportORA::loadORAFile(oraPath, gradientCreator);
    QFile::remove(oraPath);
    return result;
}
