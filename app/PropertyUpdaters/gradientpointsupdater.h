#ifndef GRADIENTPOINTSUPDATER_H
#define GRADIENTPOINTSUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class PathBox;

class GradientPointsUpdater : public PropertyUpdater {
public:
    GradientPointsUpdater(const bool &isFill, PathBox *target);
    void update();
    void frameChangeUpdate();
private:
    bool mIsFill;
    PathBox *mTarget;
};

#endif // GRADIENTPOINTSUPDATER_H
