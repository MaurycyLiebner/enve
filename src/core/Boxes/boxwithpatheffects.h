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

#ifndef BOXWITHPATHEFFECTS_H
#define BOXWITHPATHEFFECTS_H
#include "boundingbox.h"

class PathEffectAnimators;
class PathEffectsTask;
class PathEffectCaller;

class BoxWithPathEffects : public BoundingBox {
public:
    BoxWithPathEffects(const eBoxType type);

    void setupCanvasMenu(PropertyMenu * const menu);
    void prp_setupTreeViewMenu(PropertyMenu * const menu);

    void addPathEffect(const qsptr<PathEffect> &effect);
    void addFillPathEffect(const qsptr<PathEffect> &effect);
    void addOutlineBasePathEffect(const qsptr<PathEffect> &effect);
    void addOutlinePathEffect(const qsptr<PathEffect> &effect);

    void setPathEffectsEnabled(const bool enable);
    bool getPathEffectsVisible() const;

    void setFillEffectsEnabled(const bool enable);
    bool getFillEffectsVisible() const;

    void setOutlineBaseEffectsEnabled(const bool enable);
    bool getOutlineBaseEffectsVisible() const;

    void setOutlineEffectsEnabled(const bool enable);
    bool getOutlineEffectsVisible() const;

    PathEffectAnimators *getPathEffectsAnimators();
    PathEffectAnimators *getFillPathEffectsAnimators();
    PathEffectAnimators *getOutlineBasrPathEffectsAnimators();
    PathEffectAnimators *getOutlinePathEffectsAnimators();

    bool differenceInPathBetweenFrames(
            const int frame1, const int frame2) const;
    virtual bool differenceInOutlinePathBetweenFrames(
            const int frame1, const int frame2) const;
    bool differenceInFillPathBetweenFrames(
            const int frame1, const int frame2) const;

    void addBasePathEffects(const qreal relFrame,
                        QList<stdsptr<PathEffectCaller>>& list);
    void addFillEffects(const qreal relFrame,
                        QList<stdsptr<PathEffectCaller>>& list);
    void addOutlineBaseEffects(const qreal relFrame,
                               QList<stdsptr<PathEffectCaller>>& list);
    void addOutlineEffects(const qreal relFrame,
                           QList<stdsptr<PathEffectCaller>>& list);
protected:
    void getMotionBlurProperties(QList<Property*> &list) const;

    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlineBasePathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
};

#endif // BOXWITHPATHEFFECTS_H
