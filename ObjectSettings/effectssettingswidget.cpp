#include "effectssettingswidget.h"
#include <QDropEvent>
#include <QMimeData>
#include "boundingbox.h"
#include "BoxesList/complexanimatoritemwidget.h"

EffectsSettingsWidget::EffectsSettingsWidget(QWidget *parent) :
    QWidget(parent) {
    mEffectsLayout = new QVBoxLayout(this);
    mEffectsLayout->setAlignment(Qt::AlignTop);
    setLayout(mEffectsLayout);
    mDragHighlightWidget = new QLabel(this);
    mDragHighlightWidget->setFixedSize(100, 10);
    mDragHighlightWidget->setStyleSheet("background-color: black");
    mDragHighlightWidget->hide();
    setAcceptDrops(true);
}

void EffectsSettingsWidget::addWidgetForEffect(PixmapEffect *effect) {
    addWidget(new ComplexAnimatorItemWidgetContainer(effect, this));
}

void EffectsSettingsWidget::addWidget(
        ComplexAnimatorItemWidgetContainer *widget) {
    mWidgets.append(widget);
    mEffectsLayout->addWidget(widget);
}

void EffectsSettingsWidget::removeWidgetForEffect(
        PixmapEffect *effect) {
    foreach(ComplexAnimatorItemWidgetContainer *container, mWidgets) {
        if((PixmapEffect*)container->getTargetAnimator() == effect) {
            removeWidget(container);
        }
    }
}

void EffectsSettingsWidget::setBoundingBox(BoundingBox *box) {
    foreach(ComplexAnimatorItemWidgetContainer* widget, mWidgets) {
        delete widget;
    }
    mWidgets.clear();

    box->addAllEffectsToEffectsSettingsWidget(this);
}

void EffectsSettingsWidget::removeWidget(
        ComplexAnimatorItemWidgetContainer *widget) {
    mWidgets.removeOne(widget);
    delete widget;
}

void EffectsSettingsWidget::dragLeaveEvent(QDragLeaveEvent *event) {
    mDragHighlightWidget->hide();
}

void EffectsSettingsWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("effect")) {
        ComplexAnimatorItemWidget *effectWidget =
                (ComplexAnimatorItemWidget*)event->source();

        int insertY = event->pos().y();

        QWidget *widgetBefore = NULL;

        for(int i = 0; i < mEffectsLayout->count(); i++) {
            QWidget *widgetObj = mEffectsLayout->itemAt(i)->widget();
            if(widgetObj->y() > insertY) {
                break;
            }
            widgetBefore = widgetObj;
        }

        if(widgetBefore == NULL) {
            mEffectsLayout->insertWidget(0, effectWidget->parentWidget());
        } else {
            mEffectsLayout->insertWidget(
                        mEffectsLayout->indexOf(widgetBefore) + 1,
                        effectWidget->parentWidget());
        }

        event->acceptProposedAction();

        mDragHighlightWidget->hide();
    } else {
        event->ignore();
    }
}

void EffectsSettingsWidget::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasFormat("effect")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void EffectsSettingsWidget::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasFormat("effect")) {

        int insertY = event->pos().y();
        //QWidget *movedWidget = (QWidget*)event->source();

        QWidget *widgetBefore = NULL;

        for(int i = 0; i < mEffectsLayout->count(); i++) {
            QWidget *widgetObj = mEffectsLayout->itemAt(i)->widget();
            if(widgetObj->y() > insertY) {
                break;
            }
            widgetBefore = widgetObj;
        }

        if(widgetBefore == NULL) {
            insertY = 0;
        } else {
            insertY = widgetBefore->y() + widgetBefore->height();
        }
        mDragHighlightWidget->show();
        mDragHighlightWidget->move(0, insertY);

        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
