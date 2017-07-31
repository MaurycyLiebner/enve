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
    VideoBox(const QString &filePath);

    void setParent(BoxesGroup *parent);
    void setFilePath(QString path);
    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate();
    void reloadSound();

    bool SWT_isVideoBox() { return true; }
    void changeSourceFile();

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                              const int &relFrame);
    int saveToSql(QSqlQuery *query, const int &parentId);
    void loadFromSql(const int &boundingBoxId);
protected:
    void updateFrameCount(const char *path);
private:
    SingleSound *mSound = NULL;
    QString mSrcFilePath;
    int getImageAtFrame(const char *path, const int &frameId);
};

#endif // VIDEOBOX_H
