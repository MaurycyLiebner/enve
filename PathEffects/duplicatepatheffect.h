#ifndef DUPLICATEPATHEFFECT_H
#define DUPLICATEPATHEFFECT_H
#include "patheffect.h"

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    DuplicatePathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const qreal &,
                               const bool &);
    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QPointFAnimatorQSPtr mTranslation;
};

#endif // DUPLICATEPATHEFFECT_H
