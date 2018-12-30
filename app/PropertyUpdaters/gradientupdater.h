#ifndef GRADIENTUPDATER_H
#define GRADIENTUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class Gradient;

class GradientUpdater : public PropertyUpdater {
public:
    GradientUpdater(Gradient *gradient);

    void update();
    void finishedChange();

    void frameChangeUpdate();
private:
    Gradient *mTarget;
};

#endif // GRADIENTUPDATER_H
