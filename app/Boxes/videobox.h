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

    bool SWT_isVideoBox() const { return true; }
    void changeSourceFile(QWidget* dialogParent);

    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
protected:
    void updateFrameCount(const char *path);
private:
    qsptr<SingleSound> mSound;
    QString mSrcFilePath;
};

#endif // VIDEOBOX_H
