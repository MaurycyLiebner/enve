#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include "imagebox.h"
#include <unordered_map>

class SingleSound;

class VideoBox : public BoundingBox
{
    Q_OBJECT
public:
    VideoBox(const QString &filePath, BoxesGroup *parent);

    void updateAfterFrameChanged(int currentFrame);
    void draw(QPainter *p);
    void reloadPixmapIfNeeded();
    void setFilePath(QString path);
    void drawSelected(QPainter *p,
                      const CanvasMode &);
    void updateBoundingRect();
    bool relPointInsidePath(QPointF point);

    void makeDuplicate(BoundingBox *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void reloadSound();
    void reloadFile();
    void schedulePixmapReload();
    void preUpdatePixmapsUpdates();
    void reloadPixmap();
    void updateUpdateTransform();
public slots:
    void setPixmapReloadFinished();
signals:
    void pixmapReloadFinished();
protected:
    void updateFrameCount(const char *path);
private:
    std::unordered_map<int, QImage> mVideoFramesCache;
    bool mPixmapReloadScheduled = false;
    int mFramesCount = 0;
    SingleSound *mSound = NULL;
    QString mSrcFilePath;
    QImage mUpdateVideoImage;
    QImage mOldVideoImage;
    int mOldVideoFrame = 0;
    int mCurrentFrame = 0;
    int mCurrentVideoFrame = 0;
    int mUpdateVideoFrame = 0;
    int getImageAtFrame(const char *path, const int &frameId);
};

#endif // VIDEOBOX_H
