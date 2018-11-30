#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include <unordered_map>
#include "Boxes/animationbox.h"

class SingleSound;

class VideoBox : public AnimationBox {
    Q_OBJECT
public:
    VideoBox(const QString &filePath);
    VideoBox();
    ~VideoBox();

    void setParentGroup(BoxesGroup *parent);
    void setFilePath(const QString &path);
    void reloadSound();

    bool SWT_isVideoBox() { return true; }
    void changeSourceFile();

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                              const int &relFrame);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
protected:
    void updateFrameCount(const char *path);
private:
    SingleSoundQSPtr mSound;
    QString mSrcFilePath;
};

#endif // VIDEOBOX_H
