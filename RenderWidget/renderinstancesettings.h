#ifndef RENDERINSTANCESETTINGS_H
#define RENDERINSTANCESETTINGS_H
#include <QString>

class RenderInstanceSettings
{
public:
    RenderInstanceSettings();

    const QString &getName() {
        return mName;
    }

    void setName(const QString &name) {
        mName = name;
    }

private:
    QString mName;
};

#endif // RENDERINSTANCESETTINGS_H
