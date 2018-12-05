#ifndef ANIMATIONBOXFRAMEUPDATER_H
#define ANIMATIONBOXFRAMEUPDATER_H
#include "propertyupdater.h"
class AnimationBox;

class AnimationBoxFrameUpdater : public PropertyUpdater {
public:
    AnimationBoxFrameUpdater(AnimationBox *target);

    void update();
private:
    AnimationBox *mTarget;
};

#endif // ANIMATIONBOXFRAMEUPDATER_H
