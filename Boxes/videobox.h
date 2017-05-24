#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include <unordered_map>
#include "Boxes/animationbox.h"

class SingleSound;

class VideoBox : public AnimationBox
{
    Q_OBJECT
public:
    VideoBox(const QString &filePath,
             BoxesGroup *parent);

    void setFilePath(QString path);
    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void reloadSound();
    void reloadFile();
    void loadUpdatePixmap();

    bool SWT_isVideoBox() { return true; }
protected:
    void updateFrameCount(const char *path);
private:
    SingleSound *mSound = NULL;
    QString mSrcFilePath;
    int getImageAtFrame(const char *path, const int &frameId);
};

#endif // VIDEOBOX_H
