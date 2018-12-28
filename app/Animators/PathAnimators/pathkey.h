#ifndef PATHKEY_H
#define PATHKEY_H
#include "Animators/graphkey.h"
#include "pathcontainer.h"
class VectorPathAnimator;

class PathKey : public GraphKey, public PathContainer {
    friend class StdSelfRef;
public:
    NodeSettings *getNodeSettingsForPtId(const int &ptId);
    bool differsFromKey(Key *key) const { return key != this; }
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);

    stdsptr<PathKey> createNewKeyFromSubsetForPath(
            VectorPathAnimator *parentAnimator,
            const int &firstId, int count);
    void updateAfterChangedFromInside();
protected:
    PathKey(VectorPathAnimator *parentAnimator);
    PathKey(const int &relFrame,
            const SkPath &path,
            VectorPathAnimator *parentAnimator,
            const bool &closed = false);
    PathKey(const int &relFrame,
            const SkPath &path,
            const QList<SkPoint> &elementsPos,
            VectorPathAnimator *parentAnimator,
            const bool &closed);
    PathKey(const int &relFrame,
            const QList<SkPoint> &elementsPos,
            VectorPathAnimator *parentAnimator,
            const bool &closed);
};

#endif // PATHKEY_H
