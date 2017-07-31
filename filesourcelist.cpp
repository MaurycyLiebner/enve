#include "filesourcelist.h"
#include "global.h"
#include <QPainter>
#include <QScrollBar>

FileSourceWidget::FileSourceWidget(QWidget *parent) : QWidget(parent) {
    setFixedHeight(MIN_WIDGET_HEIGHT);
}

void FileSourceWidget::setTargetCache(FileCacheHandler *target) {
    mTargetCache = target;
}

void FileSourceWidget::paintEvent(QPaintEvent *) {
    if(mTargetCache == NULL || width() <= 2*MIN_WIDGET_HEIGHT) return;
    QPainter p(this);
    QString wholeString = mTargetCache->getFilePath();
    int wholeWidth = p.fontMetrics().width(wholeString);
    bool addDots = false;
    while(wholeWidth > width() - MIN_WIDGET_HEIGHT) {
        addDots = true;
        int guessLen =
                (width() - MIN_WIDGET_HEIGHT - p.fontMetrics().width("..."))*
                wholeString.count()/wholeWidth;
        wholeString = wholeString.right(guessLen);
        wholeWidth = p.fontMetrics().width("..." + wholeString);
    }
    if(addDots) {
        wholeString = "..." + wholeString;
    }

    p.drawText(rect().adjusted(0.5*MIN_WIDGET_HEIGHT, 0,
                               -0.5*MIN_WIDGET_HEIGHT, 0),
               Qt::AlignVCenter | Qt::AlignLeft,
               wholeString);

    p.end();
}

FileSourceListScrollWidget::FileSourceListScrollWidget(ScrollArea *parent) :
    MinimalScrollWidget(parent) {
    createVisiblePartWidget();
}

void FileSourceListScrollWidget::updateHeight() {
    setFixedHeight(FileSourcesCache::getFileCacheList().count() *
                   MIN_WIDGET_HEIGHT);
}

void FileSourceListScrollWidget::createVisiblePartWidget() {
    mMinimalVisiblePartWidget = new FileSourceListVisibleWidget(this);
}

FileSourceListVisibleWidget::FileSourceListVisibleWidget(MinimalScrollWidget *parent) :
    MinimalScrollWidgetVisiblePart(parent) {
    FileSourcesCache::addFileSourceListVisibleWidget(this);
}

FileSourceListVisibleWidget::~FileSourceListVisibleWidget() {
    FileSourcesCache::removeFileSourceListVisibleWidget(this);
}

void FileSourceListVisibleWidget::updateVisibleWidgetsContent() {
    int firstVisibleId = mVisibleTop/MIN_WIDGET_HEIGHT;

    const QList<FileCacheHandler*> &cacheList =
            FileSourcesCache::getFileCacheList();
            //((FileSourceListScrollWidget*)mParentWidget)->getFileCacheList();
    int idP = 0;
    for(int i = firstVisibleId;
        i < cacheList.count() &&
        idP < mSingleWidgets.count(); i++) {
        FileSourceWidget *fsw = ((FileSourceWidget*)mSingleWidgets.at(idP));
        fsw->setTargetCache(cacheList.at(i));
        fsw->show();
        idP++;
    }

    for(int i = idP; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

QWidget *FileSourceListVisibleWidget::createNewSingleWidget() {
    return new FileSourceWidget(this);
}

FileSourceList::FileSourceList(QWidget *parent) : ScrollArea(parent) {
    mScrollWidget = new FileSourceListScrollWidget(this);
    setWidget(mScrollWidget);

    connect(verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            mScrollWidget, SLOT(changeVisibleTop(int)));
    connect(this, SIGNAL(heightChanged(int)),
            mScrollWidget, SLOT(changeVisibleHeight(int)));
    connect(this, SIGNAL(widthChanged(int)),
            mScrollWidget, SLOT(setWidth(int)));

    verticalScrollBar()->setSingleStep(
                MIN_WIDGET_HEIGHT);
}
