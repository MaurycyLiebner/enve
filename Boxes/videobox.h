#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include "imagebox.h"

class SingleSound;

class VideoBox : public BoundingBox
{
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
protected:
    void updateFrameCount(const char *path);
private:
    bool mPixmapReloadScheduled = false;
    int mFramesCount = 0;
    SingleSound *mSound = NULL;
    QString mSrcFilePath;
    QImage mImage;
    int mCurrentFrame = 0;
    int getImageAtFrame(const char *path, const int &frameId);
};

#endif // VIDEOBOX_H
