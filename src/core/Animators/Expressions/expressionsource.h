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

#ifndef EXPRESSIONSOURCE_H
#define EXPRESSIONSOURCE_H
#include "expressionsourcebase.h"
#include "simpletask.h"

class ExpressionSource : public ExpressionSourceBase {
public:
    ExpressionSource(QrealAnimator* const parent);

    static sptr sCreate(const QString& path, QrealAnimator* const parent);

    qreal calculateValue(const qreal relFrame) const override;
    FrameRange identicalRange(const qreal relFrame) const override;
    QString toString() const override
    { return mPath; }

    bool dependsOn(QrealAnimator* const source) const override;
    bool isValid() const override
    { return validSource(); }

    void setPath(const QString& path);
private:
    QrealAnimator *validSource() const;

    //! @brief False when source exists, but cannot locate it, e.g.,
    //! after source was removed, but still exists in undo/redo
    void setSourcePathValid(const bool valid);

    void setSource(QrealAnimator* const newSource);
    SimpleTaskScheduler pathChanged;
    QrealAnimator* findSource();
    void reloadSource();
    void updateSourcePath();

    bool mSourcePathValid = false;
    QString mPath;
};

#endif // EXPRESSIONSOURCE_H
