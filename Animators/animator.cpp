#include "animator.h"
#include "Animators/complexanimator.h"

Animator::Animator() : QObject(), ConnectedToMainWindow()
{

}

void Animator::setParentAnimator(ComplexAnimator *parentAnimator)
{
    mParentAnimator = parentAnimator;
    if(parentAnimator == NULL) {
        clearFromGraphView();
    }
}
