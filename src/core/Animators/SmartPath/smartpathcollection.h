#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "../dynamiccomplexanimator.h"
#include "smartpathanimator.h"
#include "../../MovablePoints/segment.h"

class SmartPathAnimator;
class SmartNodePoint;
typedef DynamicComplexAnimator<SmartPathAnimator> SmartPathCollectionBase;
class SmartPathCollection : public SmartPathCollectionBase {
    e_OBJECT
protected:
    SmartPathCollection();
public:
    bool SWT_isSmartPathCollection() const { return true; }

    template<typename... Args>
    SmartPathAnimator *createNewPath(Args && ...arguments) {
        const auto newPath = enve::make_shared<SmartPathAnimator>(arguments...);
        addChild(newPath);
        return newPath.get();
    }

    SmartNodePoint * createNewSubPathAtRelPos(const QPointF &relPos);
    SmartNodePoint * createNewSubPathAtPos(const QPointF &absPos);

    void moveAllFrom(SmartPathCollection * const from) {
        const int iMax = from->ca_getNumberOfChildren() - 1;
        for(int i = iMax; i >= 0; i--)
            addChild(from->takeChildAt(i));
    }

    SkPath getPathAtRelFrame(const qreal relFrame) const;

    void applyTransform(const QMatrix &transform) const;

    void loadSkPath(const SkPath& path);

    void setFillType(const SkPath::FillType fillType) {
        mFillType = fillType;
        prp_afterWholeInfluenceRangeChanged();
    }

    SkPath::FillType getFillType() const {
        return mFillType;
    }
private:
    SkPath::FillType mFillType = SkPath::kWinding_FillType;
};

#endif // SMARTPATHCOLLECTION_H
