#include "transformable.h"
#include "bone.h"
#include "undoredo.h"

Transformable::Transformable() :
    ConnectedToMainWindow()
{

}

void Transformable::setBone(Bone *bone, bool saveUndoRedo)
{
    if(bone == (Bone*) this) return;
    if(saveUndoRedo) {
        addUndoRedo(new SetBoneUndoRedo(this, mBone, bone));
    }
    if(mBone != NULL) {
        mBone->removeTransformable(this);
    }
    if(bone != NULL) bone->addTransformable(this);
    mBone = bone;
}
