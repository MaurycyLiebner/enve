#include "singlewidgettarget.h"
#include "singlewidgetabstraction.h"
#include "scrollwidgetvisiblepart.h"

SingleWidgetTarget::SingleWidgetTarget() {
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
    return SWT_Abstraction;
}

void SingleWidgetTarget::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    Q_UNUSED(abstraction);
    Q_UNUSED(visiblePartWidget);
}
