#ifndef VALUEANIMATORS_H
#define VALUEANIMATORS_H
#include <QPointF>
#include <QList>

struct QPointFValue {
    QPointFValue(int frameT,
                 QPointF pointT,
                 bool isKeyT,
                 QPointFValue *previousKeyT,
                 QPointFValue *nextKeyT) :
        frame(frameT),
        point(pointT),
        isKey(isKeyT),
        previousKey(previousKeyT),
        nextKey(nextKeyT)
    {

    }

    int frame = 0;
    QPointF point = QPointF(0.f, 0.f);
    bool isKey = false;
    QPointFValue *previousKey = NULL;
    QPointFValue *nextKey = NULL;
};

class QPointFAnimator
{
public:
    QPointFAnimator();
    QPointF getCurrentValue();
    bool isAnimated();
    bool doesValueChangeBetweenFrames(int frame1, int frame2);
    void setCurrentValue(QPointF newValue, bool addKey);
    void saveCurrentValueAsKey();
private:
    bool mAnimated = false;
    int mCurrentFrame = 0;
    int mNumberOfFrames = 1;
    QPointF mCurrentValue = QPointF(0.f, 0.f);
    QList<QPointFValue*> mValues;
};

#endif // VALUEANIMATORS_H
