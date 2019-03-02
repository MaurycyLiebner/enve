#ifndef STROKEWIDTHUPDATER_H
#define STROKEWIDTHUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class PathBox;

class StrokeWidthUpdater : public PropertyUpdater {
public:
    StrokeWidthUpdater(PathBox *path);

    void update();

    void frameChangeUpdate();
private:
    PathBox *mTarget;
};

#endif // STROKEWIDTHUPDATER_H
