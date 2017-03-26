#include "property.h"
#include "Animators/complexanimator.h"

Property::Property() :
    QObject(),
    ConnectedToMainWindow(),
    SingleWidgetTarget() {

}

void Property::prp_setParentAnimator(ComplexAnimator *parentAnimator) {
    prp_mParentAnimator = parentAnimator;
}

void Property::prp_setZValue(const int &oldIndex,
                         const int &newIndex) {
    if(prp_mParentAnimator == NULL) return;
    prp_mParentAnimator->changeChildAnimatorZ(oldIndex, newIndex);
}

QString Property::prp_getName()
{
    return prp_mName;
}

void Property::prp_setName(QString newName) {
    prp_mName = newName;
}

void Property::prp_setUpdater(AnimatorUpdater *updater) {
    if(prp_mUpdaterBlocked) return;
    if(prp_mUpdater != NULL) prp_mUpdater->decNumberPointers();
    prp_mUpdater = updater;
    if(prp_mUpdater != NULL) prp_mUpdater->incNumberPointers();
}

void Property::prp_blockUpdater() {
    prp_mUpdaterBlocked = true;
}

void Property::prp_callUpdater() {
    if(prp_mUpdater == NULL) {
        return;
    } else {
        prp_mUpdater->update();
    }
}

void Property::prp_callFinishUpdater() {
    if(prp_mUpdater == NULL) return;
    prp_mUpdater->updateFinal();
}
