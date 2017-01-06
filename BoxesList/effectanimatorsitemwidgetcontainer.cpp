#include "effectanimatorsitemwidgetcontainer.h"

EffectAnimatorsItemWidgetContainer::EffectAnimatorsItemWidgetContainer(
        QrealAnimator *target,
        QWidget *parent) :
    ComplexAnimatorItemWidgetContainer(target, parent)
{

}

void EffectAnimatorsItemWidgetContainer::dragLeaveEvent(QDragLeaveEvent *) {
    if(mDragHighlightWidget == NULL) return;
    delete mDragHighlightWidget;
    mDragHighlightWidget = NULL;
}
#include <QMimeData>
#include "mainwindow.h"
void EffectAnimatorsItemWidgetContainer::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasFormat("effect")) {
        QByteArray itemData = event->mimeData()->data("effect");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        PixmapEffect *effectPtr = NULL;
        dataStream >> effectPtr;

        ComplexAnimatorItemWidgetContainer *effectWidget = NULL;

        foreach(ComplexAnimatorItemWidgetContainer *widget, mChildWidgets) {
            if((PixmapEffect*)widget->getTargetAnimator() == effectPtr) {
                effectWidget = widget;
                break;
            }
        }
        if(effectWidget == NULL) return;

        int insertY = event->pos().y() - BoxesListWidget::getListItemHeight();

        QWidget *widgetBefore = NULL;

        for(int i = 0; i < mChildWidgetsLayout->count(); i++) {
            QWidget *widgetObj = mChildWidgetsLayout->itemAt(i)->widget();
            if(widgetObj->y() > insertY) {
                break;
            }
            widgetBefore = widgetObj;
        }
        int widgetBeforeId = mChildWidgetsLayout->indexOf(widgetBefore);
        int effectWidgetId = mChildWidgetsLayout->indexOf(effectWidget);

        if(effectWidgetId != widgetBeforeId ||
           widgetBefore == NULL) {
            int targetId;
            if(widgetBefore == NULL) {
                targetId = 0;
            } else {
                targetId = widgetBeforeId;
                if(targetId < effectWidgetId) {
                    targetId++;
                }
            }
            effectPtr->setZValue(effectWidgetId, targetId);

            MainWindow::getInstance()->callUpdateSchedulers();
        }

        event->acceptProposedAction();

        delete mDragHighlightWidget;
        mDragHighlightWidget = NULL;
    } else {
        event->ignore();
    }
}

void EffectAnimatorsItemWidgetContainer::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat("effect")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
#include <QMimeData>
void EffectAnimatorsItemWidgetContainer::dragMoveEvent(QDragMoveEvent *event) {
    if(event->mimeData()->hasFormat("effect")) {
        int insertY = event->pos().y() - BoxesListWidget::getListItemHeight();

        QWidget *widgetBefore = NULL;

        for(int i = 0; i < mChildWidgetsLayout->count(); i++) {
            QWidget *widgetObj = mChildWidgetsLayout->itemAt(i)->widget();
            if(widgetObj->y() > insertY) {
                break;
            }
            widgetBefore = widgetObj;
        }

        if(widgetBefore == NULL) {
            insertY = BoxesListWidget::getListItemHeight();
        } else {
            insertY = widgetBefore->y() + widgetBefore->height() +
                      BoxesListWidget::getListItemHeight();
        }
        if(mDragHighlightWidget == NULL) {
            mDragHighlightWidget = new QLabel(this);
            mDragHighlightWidget->setFixedSize(100, 2);
            mDragHighlightWidget->show();
            mDragHighlightWidget->setStyleSheet("background-color: black");
        }
        mDragHighlightWidget->move(0, insertY - 2);

        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
