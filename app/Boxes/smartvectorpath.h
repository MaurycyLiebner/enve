#ifndef SMARTSmartVectorPath_H
#define SMARTSmartVectorPath_H
#include <QPainterPath>
#include <QLinearGradient>
#include "pathbox.h"
#include "Animators/SmartPath/smartpathcollection.h"

class NodePoint;
class ContainerBox;
class PathAnimator;

enum CanvasMode : short;

class SmartVectorPathEdge;

class SmartVectorPath : public PathBox {
    friend class SelfRef;
protected:
    SmartVectorPath();
public:
    bool SWT_isSmartVectorPath() const { return true; }

    void addActionsToMenu(BoxTypeMenu * const menu);

    SkPath getPathAtRelFrameF(const qreal relFrame);

    bool differenceInEditPathBetweenFrames(const int frame1,
                                           const int frame2) const;
    void applyCurrentTransformation();

    void loadSkPath(const SkPath& path);

    SmartPathCollection *getPathAnimator();

    QList<qsptr<SmartVectorPath>> breakPathsApart_k();
protected:
    void getMotionBlurProperties(QList<Property*> &list) const;
    qsptr<SmartPathCollection> mPathAnimator;
};

#endif // SMARTSmartVectorPath_H
