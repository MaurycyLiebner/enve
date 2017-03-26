#include "Animators/animatorscollection.h"
#include "Boxes/boundingbox.h"

AnimatorsCollection::AnimatorsCollection() : ComplexAnimator()
{

}

void AnimatorsCollection::setParentBoundingBox(BoundingBox *box) {
    mParentBox = box;
}

//void AnimatorsCollection::updateKeysPath()
//{
//    QrealAnimator::updateKeysPath();
//    foreach(QrealAnimator *animator, mChildAnimators) {
//        animator->updateKeysPath();
//    }
//}

//void AnimatorsCollection::sortKeys()
//{
//    QrealAnimator::sortKeys();
//    foreach(QrealAnimator *animator, mChildAnimators) {
//        animator->sortKeys();
//    }
//}

void AnimatorsCollection::childAnimatorIsRecordingChanged() {
    ComplexAnimator::childAnimatorIsRecordingChanged();
    mParentBox->SWT_scheduleWidgetsContentUpdateWithRule(SWT_Animated);
    mParentBox->SWT_scheduleWidgetsContentUpdateWithRule(SWT_NotAnimated);
}

int AnimatorsCollection::anim_getParentFrameShift() const {
    if(mParentBox == NULL) {
        return 0;
    }
    return mParentBox->getFrameShift();
}
