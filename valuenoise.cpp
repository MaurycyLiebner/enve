#include "valuenoise.h"
#include "mainwindow.h"
#include "pointhelpers.h"

ValueNoise::ValueNoise()
{
    mAmplitudeAnimator.setCurrentValue(45.);
    mAmplitudeAnimator.setName("amplitude");
    mFramePeriodAnimator.setCurrentValue(10.);
    mFramePeriodAnimator.setName("frame period");

    generateNoisePoints();
}

void ValueNoise::generateNoisePoints()
{
    int maxFrame = 200;

    qreal frame = 0.;
    bool next = true;
    while(next) {
        next = frame < maxFrame;
        qreal amplitude = mAmplitudeAnimator.getValueAtFrame(frame);
        qreal halfAmplitude = amplitude*0.5;
        mNoisePoints << NoisePoint(frame, qRandF(-halfAmplitude, halfAmplitude));
        qreal currPeriod = mFramePeriodAnimator.getValueAtFrame(frame);
        qreal nextPeriod = mFramePeriodAnimator.getValueAtFrame(frame + currPeriod);
        frame += nextPeriod;
    }
    updateCurrentValue();
}

qreal ValueNoise::getValueAtFrame(int frame) const
{
    int prevId;
    int nextId;
    getNextAndPreviousNoisePointId(&prevId, &nextId, frame);
    if(nextId == prevId) return mNoisePoints.at(nextId).value;
    NoisePoint prevNoisePoint = mNoisePoints.at(prevId);
    NoisePoint nextNoisePoint = mNoisePoints.at(nextId);
    return getValueAtFrame(frame, prevNoisePoint, nextNoisePoint);
}

qreal ValueNoise::getCurrentValue() const
{
    return mCurrentValue;
}

void ValueNoise::setFrame(int frame)
{
    if(mFrame == frame) return;
    mFrame = frame;
    updateCurrentValue();
}

void ValueNoise::updateCurrentValue()
{
    mCurrentValue = getValueAtFrame(mFrame);
}

qreal ValueNoise::getValueAtFrame(int frame,
                                  NoisePoint prevNoisePoint,
                                  NoisePoint nextNoisePoint) const
{
    qreal t = tFromX(prevNoisePoint.frame,
                     prevNoisePoint.frame,
                     nextNoisePoint.frame,
                     nextNoisePoint.frame, frame);
    qreal p0y = prevNoisePoint.value;
    qreal p1y = prevNoisePoint.value;
    qreal p2y = nextNoisePoint.value;
    qreal p3y = nextNoisePoint.value;
    return calcCubicBezierVal(p0y, p1y, p2y, p3y, t);
}


bool ValueNoise::getNextAndPreviousNoisePointId(int *prevIdP, int *nextIdP,
                                                int frame) const {
    if(mNoisePoints.isEmpty()) return false;
    int minId = 0;
    int maxId = mNoisePoints.count() - 1;
    if(frame >= mNoisePoints.last().frame) {
        *prevIdP = maxId;
        *nextIdP = maxId;
        return true;
    }
    if(frame <= mNoisePoints.first().frame) {
        *prevIdP = minId;
        *nextIdP = minId;
        return true;
    }
    while(maxId - minId > 1) {
        int guess = (maxId + minId)/2;
        qreal noisePointFrame = mNoisePoints.at(guess).frame;
        if(guess == maxId || guess == minId) {
            break;
        } else if(noisePointFrame > frame) {
            maxId = guess;
        } else {// if(noisePointFrame < frame) {
            minId = guess;
        }
    }

    if(minId == maxId) {
        NoisePoint noisePoint = mNoisePoints.at(minId);
        if(noisePoint.frame > frame) {
            if(minId != 0) {
                minId = minId - 1;
            }
        } else if(noisePoint.frame < frame) {
            if(minId < mNoisePoints.count() - 1) {
                maxId = minId + 1;
            }
        }
    }
    *prevIdP = minId;
    *nextIdP = maxId;
    return true;
}
