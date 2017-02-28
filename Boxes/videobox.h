#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include "imagebox.h"

class VideoBox : public BoundingBox
{
public:
    VideoBox(const QString &filePath, BoxesGroup *parent);

    void updateAfterFrameChanged(int currentFrame);
    void draw(QPainter *p);
    void reloadPixmap();
    void setFilePath(QString path);
    void drawSelected(QPainter *p,
                      const CanvasMode &);
    void updateBoundingRect();
    bool relPointInsidePath(QPointF point);

    void makeDuplicate(BoundingBox *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
private:
    QString mSrcFilePath;
    QImage mImage;
    int mCurrentFrame = 0;
    int getImageAtFrame(const char *path, const int &frameId, QImage *targetImg);
};

#endif // VIDEOBOX_H
