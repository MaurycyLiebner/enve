#ifndef NODEPOINTUPDATER_H
#define NODEPOINTUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class PathBox;

class NodePointUpdater : public PropertyUpdater {
public:
    NodePointUpdater(PathBox * const pathBox);

    void update();
    void frameChangeUpdate();
private:
    PathBox * const mTarget;
};

#endif // NODEPOINTUPDATER_H
