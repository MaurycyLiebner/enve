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

#include "animationcachehandler.h"

#include "CacheHandlers/imagecachecontainer.h"
#include "Private/Tasks/taskscheduler.h"
#include "Private/Tasks/complextask.h"
#include "svgexporter.h"
#include "svgexporthelpers.h"

#include <QDomElement>

AnimationFrameHandler::AnimationFrameHandler() {}

class AnimationSaverSVG : public ComplexTask {
public:
    AnimationSaverSVG(AnimationFrameHandler* const src,
                      SvgExporter& exp, QDomElement& parent,
                      const qreal div, const FrameRange& relRange) :
        ComplexTask(relRange.fMax, "SVG Paint Object"),
        mSrc(src), mExp(exp), mParent(parent),
        mRelRange(relRange), mDiv(div),
        mRelFrame(relRange.fMin - 1) {}

    void nextStep() override {
        if(!mSrc) return cancel();
        if(setValue(mRelFrame)) return finish();
        if(done()) return;

        const bool first = mFirst;
        mFirst = false;

        if(first) {
            const FrameRange idRange{qMin(mRelRange.fMin, 0), 0};
            const int span = mExp.fAbsRange.span();

            if(idRange.inRange(mRelRange) || span == 1) {
                addSurface(0, mRelRange.fMin);
                mRelFrame = mRelRange.fMax;
                return nextStep();
            }
        }

        if(mRelFrame >= mSrc->getFrameCount()) {
            mRelFrame = mRelRange.fMax;
            return nextStep();
        }
        mRelFrame++;
        if(mRelFrame >= mRelRange.fMax) return nextStep();
        if(mRelFrame >= mRelRange.fMin) {
            bool wait;
            if(mRelFrame < 0) {
                wait = addSurface(0, mRelFrame);
            } else {
                wait = addSurface(mRelFrame, mRelFrame);
            }
            if(!wait) addEmptyTask();
        } else nextStep();
    }

private:
    //! @brief Returns true if there is a task, does have to wait.
    bool addSurface(const int relFrame, const int timeFrame) {
        const QString imageId = SvgExportHelpers::ptrToStr(mSrc) +
                                QString::number(relFrame);
        const auto task = mSrc->scheduleFrameLoad(relFrame);
        if(task) {
            const QPointer<AnimationSaverSVG> ptr = this;
            task->addDependent({[ptr, relFrame, timeFrame, imageId]() {
                if(!ptr) return;
                const auto cont = ptr->mSrc->getFrameAtOrBeforeFrame(relFrame);
                if(cont) ptr->saveSurfaceValues(timeFrame, cont->getImage(), imageId);
            }, nullptr});
            addTask(task->ref<eTask>());
            return true;
        } else {
            const auto cont = mSrc->getFrameAtOrBeforeFrame(relFrame);
            if(cont) saveSurfaceValues(timeFrame, cont->getImage(), imageId);
            return false;
        }
    }

    void saveSurfaceValues(const int timeFrame, const sk_sp<SkImage>& image,
                           const QString& imageId) {
        const qreal t = (timeFrame - mRelRange.fMin)/mDiv;
        mKeyTimes << QString::number(t);

        SvgExportHelpers::defImage(mExp, image, imageId);
        mHrefValues << "#" + imageId;
    }

    void finish() {
        if(mHrefValues.isEmpty()) return;

        auto use = mExp.createElement("use");

        const QString href = mHrefValues.first();
        use.setAttribute("href", href);
        if(mHrefValues.count() > 1) {
            if(mKeyTimes.last() != "1") {
                mHrefValues << mHrefValues.last();
                mKeyTimes << "1";
            }

            const qreal dur = mDiv/mExp.fFps;
            const auto durStr = QString::number(dur)  + 's';
            const auto keyTimesStr = mKeyTimes.join(';');
            {
                auto anim = mExp.createElement("animate");
                anim.setAttribute("attributeName", "href");
                anim.setAttribute("dur", durStr);
                anim.setAttribute("values", mHrefValues.join(';'));
                anim.setAttribute("keyTimes", keyTimesStr);
                SvgExportHelpers::assignLoop(anim, mExp.fLoop);
                use.appendChild(anim);
            }
        }

        mParent.appendChild(use);
    }

    const QPointer<AnimationFrameHandler> mSrc;
    SvgExporter& mExp;
    QDomElement& mParent;
    const FrameRange mRelRange;
    const qreal mDiv;

    bool mFirst = true;
    int mRelFrame = 0;

    QStringList mHrefValues;
    QStringList mKeyTimes;
};

eTaskBase* AnimationFrameHandler::saveAnimationSVG(
        SvgExporter& exp, QDomElement& parent,
        const FrameRange& relRange) {
    const int span = exp.fAbsRange.span();
    const qreal div = span - 1;

    const auto task = new AnimationSaverSVG(this, exp, parent, div, relRange);
    const auto taskSPtr = QSharedPointer<AnimationSaverSVG>(
                              task, &QObject::deleteLater);
    task->nextStep();

    if(task->done()) return nullptr;
    TaskScheduler::instance()->addComplexTask(taskSPtr);
    return task;
}
