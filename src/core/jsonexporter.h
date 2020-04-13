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

#ifndef JSONEXPORTER_H
#define JSONEXPORTER_H

#include "Private/Tasks/complextask.h"

#include "framerange.h"

#include <QJsonDocument>

class Canvas;

enum class JsonLayerType {
    preComp,
    solidColor,
    image,
    null,
    shape,
    text
};

class CORE_EXPORT JsonExporter : public ComplexTask {
public:
    JsonExporter(const QString& path, Canvas* const scene,
                 const FrameRange& frameRange,
                 const qreal fps, const bool background);

    void nextStep() override;

    void addNextTask(const stdsptr<eTask>& task);

    void addLayer(QJsonObject&& layer);
    void addAsset(QJsonValue&& asset)
    { mAssets.append(asset); }

    QJsonObject createLayer(const JsonLayerType type) const;

    Canvas* const fScene;
    const FrameRange fAbsRange;
    const qreal fFps;
    const bool fBackground;
private:
    void finish();

    bool mStarted = false;
    QString mPath;
    QJsonArray mLayers;
    QJsonArray mAssets;
    QList<stdsptr<eTask>> mWaitingTasks;
};

#endif // JSONEXPORTER_H
