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

#include "jsonexporter.h"

#include "canvas.h"

JsonExporter::JsonExporter(const QString &path, Canvas * const scene,
                           const FrameRange &frameRange,
                           const qreal fps, const bool background) :
    ComplexTask(INT_MAX, "JSON Export"),
    fScene(scene), fAbsRange(frameRange), fFps(fps),
    fBackground(background),
    mPath(path) {}

void JsonExporter::nextStep() {
    if(!mStarted) {
        fScene->saveSceneJSON(*this);
        mStarted = true;
    }
    if(mWaitingTasks.isEmpty()) return finish();
    const auto task = mWaitingTasks.takeFirst();
    addTask(task);
}

void JsonExporter::addNextTask(const stdsptr<eTask>& task) {
    mWaitingTasks << task;
}

void JsonExporter::addLayer(QJsonObject &&layer) {
    const int ind = mLayers.count() + 1;
    layer.insert("ind", QJsonValue(ind));
    mLayers.append(layer);
}

QJsonObject JsonExporter::createLayer(const JsonLayerType type) const {
    const int typeId = static_cast<int>(type);
    QJsonObject result(
    {
        qMakePair(QString("ao"), QJsonValue(0)),
        qMakePair(QString("ddd"), QJsonValue(0)),
        qMakePair(QString("sr"), QJsonValue(1)),
        qMakePair(QString("st"), QJsonValue(0)),
        qMakePair(QString("ty"), QJsonValue(typeId))
    });

    return result;
}

void JsonExporter::finish() {
    const int w = fScene->getCanvasWidth();
    const int h = fScene->getCanvasHeight();
    QJsonDocument doc;
    QJsonObject docObj(
    {
        qMakePair(QString("nm"), QJsonValue("enve")),
        qMakePair(QString("v"), QJsonValue("0.0.0")),
        qMakePair(QString("ddd"), QJsonValue(0)),
        qMakePair(QString("ip"), QJsonValue(0)),
        qMakePair(QString("op"), QJsonValue(fAbsRange.span() - 1)),
        qMakePair(QString("w"), QJsonValue(w)),
        qMakePair(QString("h"), QJsonValue(h)),
        qMakePair(QString("fr"), QJsonValue(fFps)),
        qMakePair(QString("assets"), QJsonValue(mAssets)),
        qMakePair(QString("layers"), QJsonValue(mLayers))
    });
    doc.setObject(docObj);
    QFile file(mPath);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        RuntimeThrow("Could not open:\n\"" + file.fileName() + "\"");
    }
    setValue(INT_MAX);
}
