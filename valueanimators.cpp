#include "valueanimators.h"

QPointFAnimator::QPointFAnimator()
{

}

QPointF QPointFAnimator::getCurrentValue()
{
    return mCurrentValue;
}

bool QPointFAnimator::isAnimated()
{
    return mAnimated;
}

bool QPointFAnimator::doesValueChangeBetweenFrames(int frame1, int frame2)
{
    if (mAnimated) {
        return mValues.at(frame1)->point != mValues.at(frame2)->point;
    } else{
        return false;
    }
}

void QPointFAnimator::setCurrentValue(QPointF newValue, bool addKey)
{
    mCurrentValue = newValue;
    if(addKey) {
        saveCurrentValueAsKey();
    }

}

void QPointFAnimator::saveCurrentValueAsKey()
{
    if(!mAnimated) {
        mAnimated = true;
        for(int i = 0; i < mNumberOfFrames; i++) {
            mValues.append(new QPointFValue(i, mCurrentValue,
                                            false, NULL, NULL));
        }
    }
    QPointFValue *valueToChange = mValues.at(mCurrentFrame);

    QPointFValue *prevKey = valueToChange->previousKey;
    int firstFrameToUpdate;
    if(prevKey == NULL) {
        firstFrameToUpdate = -1;
    } else {
        firstFrameToUpdate = prevKey->frame;
    }

    QPointFValue *nextKey = valueToChange->nextKey;
    int lastFrameToUpdate;
    if(nextKey == NULL) {
        lastFrameToUpdate = mNumberOfFrames;
    } else {
        lastFrameToUpdate = nextKey->frame;
    }
    valueToChange->point = mCurrentValue;
    valueToChange->isKey = true;

    if(nextKey == NULL) {
        for(int i = mCurrentFrame + 1; i < lastFrameToUpdate; i++) {
            QPointFValue *iValue = mValues.at(i);
            iValue->previousKey = valueToChange;
            iValue->point = mCurrentValue;
        }
    } else {
        QPointF nextKeyValue = nextKey->point;
        int distBetweenKeys = lastFrameToUpdate - mCurrentFrame;
        for(int i = mCurrentFrame + 1; i < lastFrameToUpdate; i++) {
            QPointFValue *iValue = mValues.at(i);
            iValue->previousKey = valueToChange;
            iValue->point = ( mCurrentValue*(lastFrameToUpdate - i) +
                            nextKeyValue*(i - mCurrentFrame) ) /
                            distBetweenKeys;
        }
    }

    if(prevKey == NULL) {
        for(int i = mCurrentFrame - 1; i > firstFrameToUpdate; i--) {
            QPointFValue *iValue = mValues.at(i);
            iValue->nextKey = valueToChange;
            iValue->point = mCurrentValue;
        }
    } else {
        QPointF prevKeyValue = nextKey->point;
        int distBetweenKeys = firstFrameToUpdate - mCurrentFrame;
        for(int i = mCurrentFrame - 1; i > firstFrameToUpdate; i--) {
            QPointFValue *iValue = mValues.at(i);
            iValue->previousKey = valueToChange;
            iValue->point = ( mCurrentValue*(firstFrameToUpdate - i) +
                            prevKeyValue*(i - mCurrentFrame) ) /
                            distBetweenKeys;
        }
    }
}
