#ifndef FILESOURCELIST_H
#define FILESOURCELIST_H
#include "FileCacheHandlers/filecachehandler.h"
#include "filesourcescache.h"
#include <QWidget>
#include "GUI/BoxesList/OptimalScrollArea/minimalscrollwidgetvisiblepart.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "GUI/BoxesList/OptimalScrollArea/minimalscrollwidget.h"
class FileSourceListVisibleWidget;
struct FileCacheHandlerAbstraction {
    FileCacheHandlerAbstraction(
            FileCacheHandler *targetT,
            FileSourceListVisibleWidget *parentVisibleWidgetT) {
        fTarget = targetT;
        parentVisibleWidget = parentVisibleWidgetT;
    }

    FileCacheHandler *fTarget;
    bool selected = false;

    void switchSelected() {
        setSelected(!selected);
    }

    void setSelected(const bool bT);

    const QString &getName() {
        return fTarget->path();
    }

    bool isFileMissing() {
        return fTarget->fileMissing();
    }

    FileSourceListVisibleWidget *parentVisibleWidget;
};

class FileSourceWidget : public QWidget {
    Q_OBJECT
public:
    FileSourceWidget(FileSourceListVisibleWidget *parent = nullptr);

    void setTargetCache(FileCacheHandlerAbstraction *target);

    void switchFileNameOnly();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
    bool mFileNameOnly = true;
    FileCacheHandlerAbstraction *mTargetCache = nullptr;
    FileSourceListVisibleWidget *mParentVisibleWidget = nullptr;
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
    FileSourceListVisibleWidget(MinimalScrollWidget *parent = nullptr);
    ~FileSourceListVisibleWidget();

    void updateVisibleWidgetsContent();

    QWidget *createNewSingleWidget();
    void addCacheHandlerToList(FileCacheHandler * const handler);
    void removeCacheHandlerFromList(FileCacheHandler * const handler);

    void addToSelectedList(FileCacheHandlerAbstraction *item) {
        mSelectedList << item;
        update();
    }

    void removeFromSelectedList(FileCacheHandlerAbstraction *item) {
        mSelectedList.removeOne(item);
        update();
    }

    void clear() {
        clearSelected();
        mCacheList.clear();
    }

    void clearSelected() {
        for(const auto& abs : mSelectedList) {
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
    static QList<FileSourceListVisibleWidget*> sWidgets;

    QList<FileCacheHandlerAbstraction*> mSelectedList;
    QList<stdsptr<FileCacheHandlerAbstraction>> mCacheList;
    void paintEvent(QPaintEvent *);
};

class FileSourceList : public ScrollArea {
    Q_OBJECT
public:
    FileSourceList(QWidget *parent = nullptr);

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
private:
    FileSourceListScrollWidget *mScrollWidget;
};

#endif // FILESOURCELIST_H
