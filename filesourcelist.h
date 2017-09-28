#ifndef FILESOURCELIST_H
#define FILESOURCELIST_H
#include <QWidget>
#include "filesourcescache.h"
#include "BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"
#include "BoxesList/OptimalScrollArea/minimalscrollwidget.h"

struct FileCacheHandlerAbstraction {
    FileCacheHandlerAbstraction(
            FileCacheHandler *targetT,
            FileSourceListVisibleWidget *parentVisibleWidgetT) {
        target = targetT;
        parentVisibleWidget = parentVisibleWidgetT;
    }

    FileCacheHandler *target;
    bool selected = false;

    void switchSelected() {
        setSelected(!selected);
    }

    void setSelected(const bool &bT);

    const QString &getFilePath() {
        return target->getFilePath();
    }

    FileSourceListVisibleWidget *parentVisibleWidget;
};

class FileSourceWidget : public QWidget {
    Q_OBJECT
public:
    FileSourceWidget(FileSourceListVisibleWidget *parent = NULL);

    void setTargetCache(FileCacheHandlerAbstraction *target);

    void switchFileNameOnly();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
    bool mFileNameOnly = true;
    FileCacheHandlerAbstraction *mTargetCache = NULL;
    FileSourceListVisibleWidget *mParentVisibleWidget = NULL;
    void mouseMoveEvent(QMouseEvent *event);
    QPoint mDragStartPos;
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
    void addCacheHandlerToList(FileCacheHandler *handler);

    void removeCacheHandlerFromList(FileCacheHandler *handler);

    void addToSelectedList(FileCacheHandlerAbstraction *item) {
        mSelectedList << item;
        update();
    }

    void removeFromSelectedList(FileCacheHandlerAbstraction *item) {
        mSelectedList.removeOne(item);
        update();
    }

    void clearSelected() {
        foreach(FileCacheHandlerAbstraction *abs,
                mSelectedList) {
            abs->selected = false;
        }

        mSelectedList.clear();
        update();
    }

    void showContextMenu(const QPoint &globalPos);

    int getCacheListCount() {
        return mCacheList.count();
    }
protected:
    QList<FileCacheHandlerAbstraction*> mSelectedList;
    QList<FileCacheHandlerAbstraction*> mCacheList;
    void paintEvent(QPaintEvent *);
};

class FileSourceList : public ScrollArea {
    Q_OBJECT
public:
    FileSourceList(QWidget *parent = NULL);

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
private:
    FileSourceListScrollWidget *mScrollWidget;
};

#endif // FILESOURCELIST_H
