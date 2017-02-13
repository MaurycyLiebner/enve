#include "singlewidgettarget.h"
#include "singlewidgetabstraction.h"
#include "scrollwidgetvisiblepart.h"

SingleWidgetTarget::SingleWidgetTarget() {
}

SingleWidgetTarget::~SingleWidgetTarget() {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        delete abs;
    }
}

//void SingleWidgetTarget::SWT_addChildrenAbstractions(
//        SingleWidgetAbstraction *SWT_Abstraction,
//        ScrollWidgetVisiblePart *visiblePartWidget) {
//    foreach(SingleWidgetTarget *child, mChildren) {
//        SWT_Abstraction->addChildAbstraction(
//                    child->createAbstraction(visiblePartWidget));
//    }
//}

SingleWidgetAbstraction *SingleWidgetTarget::SWT_createAbstraction(
        ScrollWidgetVisiblePart *visiblePartWidget) {
    SingleWidgetAbstraction *SWT_Abstraction =
            new SingleWidgetAbstraction(this,
                                        visiblePartWidget);
    SWT_addChildrenAbstractions(SWT_Abstraction,
                                visiblePartWidget);
    mSWT_allAbstractions << SWT_Abstraction;
    return SWT_Abstraction;
}

void SingleWidgetTarget::SWT_removeAbstractionFromList(
        SingleWidgetAbstraction *abs) {
    mSWT_allAbstractions.removeOne(abs);
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAll(
        SingleWidgetTarget *target) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->addChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAllAt(
        SingleWidgetTarget *target, const int &id) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->addChildAbstractionForTargetAt(target, id);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithRule(
        const SWT_Rule &rule) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfIsCurrentRule(rule);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithTarget(
        SingleWidgetTarget *targetP,
        const SWT_Target &target) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfIsCurrentTarget(targetP,
                                                          target);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty() {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfSearchNotEmpty();
    }
}

void SingleWidgetTarget::SWT_removeChildAbstractionForTargetFromAll(
        SingleWidgetTarget *target) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->removeChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_moveChildAbstractionForTargetToInAll(
        SingleWidgetTarget *target, const int &id) {
    foreach(SingleWidgetAbstraction *abs, mSWT_allAbstractions) {
        abs->moveChildAbstractionForTargetTo(target, id);
    }
}
