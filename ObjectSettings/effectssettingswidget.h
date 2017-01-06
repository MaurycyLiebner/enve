#ifndef EFFECTSSETTINGSWIDGET_H
#define EFFECTSSETTINGSWIDGET_H

#include <QLabel>
#include <QVBoxLayout>
#include "BoxesList/complexanimatoritemwidgetcontainer.h"
#include "pixmapeffect.h"

class EffectsSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EffectsSettingsWidget(QWidget *parent = 0);

    void addWidgetForEffect(PixmapEffect *effect);
    void addWidget(ComplexAnimatorItemWidgetContainer *widget);
    void removeWidgetForEffect(PixmapEffect *effect);
    void removeWidget(ComplexAnimatorItemWidgetContainer *widget);

    void dragLeaveEvent(QDragLeaveEvent *);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void setBoundingBox(BoundingBox *box);
    void dropEvent(QDropEvent *event);
protected:
    BoundingBox *mCurrentBox = NULL;
    QVBoxLayout *mEffectsLayout;
    QLabel *mDragHighlightWidget;

    QList<ComplexAnimatorItemWidgetContainer*> mWidgets;
signals:

public slots:
};

#endif // EFFECTSSETTINGSWIDGET_H
