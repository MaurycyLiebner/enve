#ifndef LINKBOX_H
#define LINKBOX_H
#include "boxesgroup.h"

class LinkBox : public BoxesGroup
{
public:
    LinkBox(QString srcFile, BoxesGroup *parent);
    void reload();

    void changeSrc();

    void setSrc(const QString &src);

private:
    QString mSrc;
};

#endif // LINKBOX_H
