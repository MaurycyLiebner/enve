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

#ifndef INTERPOLATIONANIMATORT_H
#define INTERPOLATIONANIMATORT_H
#include "graphanimatort.h"
#include "qrealpoint.h"
#include "interpolationkeyt.h"

template <typename T, typename K = InterpolationKeyT<T>>
class InterpolationAnimatorT :
        public BasedAnimatorT<GraphAnimator, K, T> {
    e_OBJECT
public:
    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType type,
            qreal &minValue, qreal &maxValue) const override {
        if(type == QrealPointType::keyPt) {
            minValue = key->getRelFrame();
            maxValue = minValue;
            //getFrameConstraints(key, type, minValue, maxValue);
        } else {
            minValue = -DBL_MAX;
            maxValue = DBL_MAX;
        }
    }
protected:
    InterpolationAnimatorT(const QString& name) :
        BasedAnimatorT<GraphAnimator, K, T>(name) {}

    T getValueAtRelFrameK(const qreal frame,
                          const K * const prevKey,
                          const K * const nextKey) const override;

    using StringToValue = std::function<void(T&, QStringRef)>;
    void readValuesXEV(const QDomElement& ele, const StringToValue& strToVal);
    using ValueToString = std::function<QString(const T&)>;
    void writeValuesXEV(QDomElement& ele, const ValueToString& valToStr) const;
};

template<typename T, typename K>
T InterpolationAnimatorT<T, K>::getValueAtRelFrameK(
        const qreal frame, const K* const prevKey,
        const K* const nextKey) const {
    T result;
    const qreal prevFrame = prevKey->getRelFrame();
    const qreal nextFrame = nextKey->getRelFrame();
    const qCubicSegment1D seg{prevFrame,
                prevKey->getC1Frame(),
                nextKey->getC0Frame(),
                nextFrame};
    const qreal t = gTFromX(seg, frame);
    const qreal p0y = prevKey->getValueForGraph();
    const qreal p1y = prevKey->getC1Value();
    const qreal p2y = nextKey->getC0Value();
    const qreal p3y = nextKey->getValueForGraph();
    const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
    const qreal tEff = (iFrame - prevFrame)/(nextFrame - prevFrame);
    gInterpolate(prevKey->getValue(), nextKey->getValue(), tEff, result);
    return result;
}

template<typename T, typename K>
void InterpolationAnimatorT<T, K>::readValuesXEV(
        const QDomElement& ele, const StringToValue& strToVal) {
    const bool hasValues = ele.hasAttribute("values");
    const bool hasFrames = ele.hasAttribute("frames");
    const bool hasKeys = hasValues && hasFrames;
    if(hasKeys) {
        const QString valueStrs = ele.attribute("values");
        const QString frameStrs = ele.attribute("frames");

        const auto values = valueStrs.splitRef(';');
        const auto framess = frameStrs.splitRef(';');
        if(values.count() != framess.count())
            RuntimeThrow("The values count does not match the frames count");
        const int iMax = values.count();
        for(int i = 0; i < iMax; i++) {
            const auto& value = values[i];
            const auto frames = framess[i].split(' ');
            if(frames.count() != 3) {
                RuntimeThrow("Invalid frames count " + framess[i].toString());
            }

            const int frame = XmlExportHelpers::stringToInt(frames[1]);
            const auto key = enve::make_shared<K>(frame, this);
            auto& keyValue = key->getValue();
            strToVal(keyValue, value);
            key->setC0Frame(XmlExportHelpers::stringToDouble(frames[0]));
            key->setC1Frame(XmlExportHelpers::stringToDouble(frames[2]));
            this->anim_appendKey(key);
        }
    } else if(ele.hasAttribute("value")) {
        const QString value = ele.attribute("value");
        strToVal(this->mCurrentValue, &value);
    } else RuntimeThrow("No values/frames and no value provided");
}

template<typename T, typename K>
void InterpolationAnimatorT<T, K>::writeValuesXEV(
        QDomElement& ele, const ValueToString& valToStr) const {
    if(this->anim_hasKeys()) {
        QString values;
        QString frames;
        const QString blueprint = QStringLiteral("%1 %2 %3");
        const auto& keys = this->anim_getKeys();
        for(const auto &key : keys) {
            const auto smKey = static_cast<K*>(key);
            const QString v = valToStr(smKey->getValue());

            const qreal fc0 = smKey->getC0Frame();
            const int f = smKey->getRelFrame();
            const qreal fc2 = smKey->getC1Frame();

            if(!values.isEmpty()) values += ';';
            values += v;
            if(!frames.isEmpty()) frames += ';';
            frames += blueprint.arg(fc0).arg(f).arg(fc2);
        }
        ele.setAttribute("values", values);
        ele.setAttribute("frames", frames);
    } else ele.setAttribute("value", valToStr(this->mCurrentValue));
}

#endif // INTERPOLATIONANIMATORT_H
