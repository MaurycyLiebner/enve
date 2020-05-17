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

#ifndef SCULPTPATHCOLLECTION_H
#define SCULPTPATHCOLLECTION_H
#include <QList>
#include "../dynamiccomplexanimator.h"
#include "sculptpathanimator.h"

typedef DynamicComplexAnimator<SculptPathAnimator> SculptPathCollectionBase;

class CORE_EXPORT SculptPathCollection : public SculptPathCollectionBase {
    Q_OBJECT
    e_OBJECT
protected:
    SculptPathCollection();
public:
    void prp_writeProperty_impl(eWriteStream& dst) const;
    void prp_readProperty_impl(eReadStream& src);

    QString prp_tagNameXEV() const { return "SculptPaths"; }

    void sculpt(const SculptTarget target,
                const SculptMode mode,
                const SculptBrush &brush);

    SculptPathAnimator* createNewSubPathAtRelPos(
            const QPointF &relPos, const qreal radius);
    SculptPathAnimator* createNewSubPathAtPos(
            const QPointF &absPos, const qreal radius);

    QList<SculptPath> getPathAtRelFrame(const qreal relFrame) const;

    void applyTransform(const QMatrix &transform) const;

    void loadSkPath(const SkPath& path);

    void setFillType(const SkPathFillType fillType);
    SkPathFillType getFillType() const
    { return mFillType; }
signals:
    void fillTypeChanged(SkPathFillType);
private:
    void updateVisibleChildren();

    SculptPathAnimator *createNewPath();
    SculptPathAnimator *createNewPath(const SkPath& path);

    //void updatePathColors();

    SkPathFillType mFillType = SkPathFillType::kWinding;
};

#endif // SCULPTPATHCOLLECTION_H
