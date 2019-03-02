#ifndef NODEPOINTUPDATER_H
#define NODEPOINTUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class PathBox;

class NodePointUpdater : public PropertyUpdater {
public:
    NodePointUpdater(PathBox *vectorPath);

    void update();

    void frameChangeUpdate();
private:
    PathBox *mTarget;
};

#endif // NODEPOINTUPDATER_H
