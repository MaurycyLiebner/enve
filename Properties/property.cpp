#include "property.h"
#include "Animators/complexanimator.h"

Property::Property() :
    QObject(),
    ConnectedToMainWindow(),
    SingleWidgetTarget() {

}

void Property::setParentAnimator(ComplexAnimator *parentAnimator) {
    mParentAnimator = parentAnimator;
}

void Property::setZValue(const int &oldIndex,
                         const int &newIndex) {
    if(mParentAnimator == NULL) return;
    mParentAnimator->changeChildAnimatorZ(oldIndex, newIndex);
}

QString Property::getName()
{
    return mName;
}

void Property::setName(QString newName)
{
    mName = newName;
}
