#ifndef RENDERCACHEHANDLER_H
#define RENDERCACHEHANDLER_H
#include <QtCore>
class QPainter;
class RenderContainer;
class RenderContainer;
class CacheContainer;
class RenderCacheHandler;

class FileCacheHandler {
public:
    FileCacheHandler();

    const QString &getFilePath() {
        return mFilePath;
    }

private:
    QString mFilePath;
};

class CacheHandler : public QObject {
    Q_OBJECT
public:
    void removeRenderContainer(CacheContainer *cont);
    CacheContainer *getRenderContainerAtRelFrame(const int &frame);
    virtual CacheContainer *createNewRenderContainerAtRelFrame(const int &frame);
    int getFirstEmptyFrameAtOrAfterFrame(const int &frame);
    void setContainersInFrameRangeBlocked(const int &minFrame,
                                          const int &maxFrame,
                                          const bool &blocked);

    void drawCacheOnTimeline(QPainter *p,
                             const qreal &pixelsPerFrame,
                             const qreal &drawY,
                             const int &startFrame,
                             const int &endFrame);
    void clearCache();
    CacheContainer *getRenderContainerAtOrBeforeRelFrame(const int &frame);
    int getFirstEmptyFrameAfterFrame(const int &frame);
protected:
    int getRenderContainterInsertIdAtRelFrame(const int &relFrame);
    bool getRenderContainterIdAtRelFrame(const int &relFrame, int *id);
    QList<CacheContainer*> mRenderContainers;
};

class BoundingBox;
class RenderCacheHandler : public CacheHandler {
    Q_OBJECT
public:
    RenderCacheHandler();

    CacheContainer *createNewRenderContainerAtRelFrame(const int &frame);

    void updateCurrentRenderContainerFromFrame(const int &relFrame);

    bool updateCurrentRenderContainerFromFrameIfNotNull(const int &relFrame);

    void duplicateCurrentRenderContainerFrom(RenderContainer *cont);

    void updateCurrentRenderContainerTransform(const QMatrix &trans);

    void drawCurrentRenderContainer(QPainter *p);

    void clearCacheForRelFrameRange(const int &minFrame,
                                    const int &maxFrame);
    void clearCacheForAbsFrameRange(const int &minFrame,
                                    const int &maxFrame);

    void setParentBox(BoundingBox *parentBox) {
        mParentBox = parentBox;
    }

    void relRangeToAbsRange(int *minFrame,
                            int *maxFrame) {
        *minFrame = relFrameToAbsFrame(*minFrame);
        *maxFrame = relFrameToAbsFrame(*maxFrame);
    }

    void absRangeToRelRange(int *minFrame,
                            int *maxFrame) {
        *minFrame = absFrameToRelFrame(*minFrame);
        *maxFrame = absFrameToRelFrame(*maxFrame);
    }

    int relFrameToAbsFrame(const int &relFrame);
    int absFrameToRelFrame(const int &absFrame);
signals:
    void clearedCacheForAbsFrameRange(int, int);
public slots:
private:
    BoundingBox *mParentBox = NULL;
};

#endif // RENDERCACHEHANDLER_H
