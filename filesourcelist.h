#ifndef FILESOURCELIST_H
#define FILESOURCELIST_H
#include <QWidget>
#include "filesourcescache.h"
#include "BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "BoxesList/OptimalScrollArea/minimalscrollwidget.h"

class FileSourceWidget : public QWidget {
    Q_OBJECT
public:
    FileSourceWidget(QWidget *parent = NULL);

    void setTargetCache(FileCacheHandler *target);

    void paintEvent(QPaintEvent *);

private:
    FileCacheHandler *mTargetCache = NULL;
};

class FileSourceListScrollWidget : public MinimalScrollWidget {
    Q_OBJECT
public:
    FileSourceListScrollWidget(ScrollArea *parent);

    void updateHeight();

    void createVisiblePartWidget();
};

class FileSourceListVisibleWidget : public MinimalScrollWidgetVisiblePart {
    Q_OBJECT
public:
    FileSourceListVisibleWidget(MinimalScrollWidget *parent = 0);
    ~FileSourceListVisibleWidget();

    void updateVisibleWidgetsContent();

    QWidget *createNewSingleWidget();
};

class FileSourceList : public ScrollArea {
    Q_OBJECT
public:
    FileSourceList(QWidget *parent = NULL);

private:
    FileSourceListScrollWidget *mScrollWidget;
};

#endif // FILESOURCELIST_H
