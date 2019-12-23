// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#include "elinearize.h"
#include "enveCore/Animators/qrealanimator.h"

qsptr<CustomPathEffect> eCreateNewestVersion() {
    // Use default, most up to date, version
    return enve::make_shared<eLinearize000>();
}

qsptr<CustomPathEffect> eCreate(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier)
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return enve::make_shared<eLinearize000>();
}

// Returned value must be unique, lets enve distinguish effects
QString effectId() {
    return "waer9yv11r3gl10x1qtm";
}

// Name of your effect used in UI
QString eName() {
    return "eLinearize";
}

// here specify your effect's most up to date version
CustomIdentifier::Version effectVersion() {
    return { 0, 0, 0 };
}

CustomIdentifier eIdentifier() {
    return { effectId(), eName(), effectVersion() };
}

bool eSupports(const CustomIdentifier &identifier) {
    if(identifier.fEffectId != effectId()) return false;
    if(identifier.fEffectName != eName()) return false;
    return identifier.fVersion == effectVersion();
}

eLinearize000::eLinearize000() :
    CustomPathEffect(eName().toLower()) {
    mInfluence = enve::make_shared<QrealAnimator>(0, 0, 1, 0.1, "influence");
    ca_addChild(mInfluence);
}

CustomIdentifier eLinearize000::getIdentifier() const {
    return { effectId(), eName(), { 0, 0, 0 } };
}

class eLinearize000EffectCaller : public PathEffectCaller {
public:
    eLinearize000EffectCaller(const qreal infl) :
        mInfl(toSkScalar(infl)) {}

    void apply(SkPath& path);
private:
    const float mInfl;
};

void eLinearize000EffectCaller::apply(SkPath &path) {
    const float invInf = 1 - mInfl;
    SkPath src;
    path.swap(src);
    path.setFillType(src.getFillType());

    SkPath::Iter iter(src, false);
    SkPoint pts[4];
    for(;;) {
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            path.lineTo(pts[1]);
        } break;
        case SkPath::kQuad_Verb: {
            path.quadTo(pts[1]*invInf + (pts[0] + pts[2])*0.5f*mInfl, pts[2]);
        } break;
        case SkPath::kConic_Verb: {
            path.conicTo(pts[1]*invInf + (pts[0] + pts[2])*0.5f*mInfl,
                         pts[2], iter.conicWeight());
        } break;
        case SkPath::kCubic_Verb: {
            path.cubicTo(pts[0]*mInfl + pts[1]*invInf,
                         pts[3]*mInfl + pts[2]*invInf, pts[3]);
        } break;
        case SkPath::kClose_Verb: {
            path.close();
        } break;
        case SkPath::kMove_Verb: {
            path.moveTo(pts[0]);
        } break;
        case SkPath::kDone_Verb:
            return;
        }
    }
}

stdsptr<PathEffectCaller> eLinearize000::getEffectCaller(const qreal relFrame) const {
    const qreal infl = mInfluence->getEffectiveValue(relFrame);
    return enve::make_shared<eLinearize000EffectCaller>(infl);
}
