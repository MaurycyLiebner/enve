#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"
#include "Boxes/vectorpath.h"
#include "fillstrokesettings.h"

#define Q_FOREACHBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class MainWindow;

class VectorPathEdge;

class BoxesGroup : public BoundingBox
{
    Q_OBJECT
public:
    BoxesGroup(BoxesGroup *parent);
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting);
    ~BoxesGroup();
    virtual void prp_loadFromSql(const int &boundingBoxId);

    BoundingBox *createLink(BoxesGroup *parent);
    BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();

    BoundingBox *getBoxAt(const QPointF &absPos);
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);

    void addContainedBoxesToSelection(QRectF rect);

    void setIsCurrentGroup(bool bT);

    BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    void setFillSettings(PaintSettings fillSettings,
                         bool saveUndoRedo = true);
    void setStrokeSettings(StrokeSettings strokeSettings,
                           bool saveUndoRedo = true);

    void setCurrentFillStrokeSettingsFromBox(BoundingBox *box);

    void ungroup();

    int prp_saveToSql(QSqlQuery *query, const int &parentId);
    BoxesGroup *loadChildrenFromSql(int thisBoundingBoxId, bool loadInBox);

    bool isCurrentGroup();
    void addChild(BoundingBox *child);
    void addChildToListAt(int index, BoundingBox *child, bool saveUndoRedo = true);
    void updateChildrenId(int firstId, bool saveUndoRedo = true);
    void updateChildrenId(int firstId, int lastId, bool saveUndoRedo = true);
    void removeChild(BoundingBox *child);
    void increaseChildZInList(BoundingBox *child);
    void decreaseChildZInList(BoundingBox *child);
    void bringChildToEndList(BoundingBox *child);
    void bringChildToFrontList(BoundingBox *child);
    void moveChildInList(BoundingBox *child,
                         int from, int to,
                         bool saveUndoRedo = true);
    void moveChildBelow(BoundingBox *boxToMove,
                        BoundingBox *below);
    void moveChildAbove(BoundingBox *boxToMove,
                        BoundingBox *above);

    void removeChildFromList(int id, bool saveUndoRedo = true);

    void updateAfterFrameChanged(const int &currentFrame);


    void setFillGradient(Gradient* gradient, bool finish);
    void setStrokeGradient(Gradient* gradient, bool finish);
    void setFillFlatColor(Color color, bool finish);
    void setStrokeFlatColor(Color color, bool finish);

    void setStrokeCapStyle(Qt::PenCapStyle capStyle);
    void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle);
    void setStrokeWidth(qreal strokeWidth, bool finish);

    PaintSettings *getFillSettings();
    StrokeSettings *getStrokeSettings();

    static bool getCtrlsAlwaysVisible();
    static void setCtrlsAlwaysVisible(bool bT);

    void updateAllBoxes();

    void updateRelBoundingRect();
    void applyCurrentTransformation();

    bool relPointInsidePath(const QPointF &relPos);
//    QPointF getRelCenterPosition();
    void updateEffectsMargin();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    void applyPaintSetting(
            const PaintSetting &setting) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            box->applyPaintSetting(setting);
        }
    }

    void setFillColorMode(const ColorMode &colorMode) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box,  mChildBoxes) {
            box->setFillColorMode(colorMode);
        }
    }
    void setStrokeColorMode(const ColorMode &colorMode) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mChildBoxes) {
            box->setStrokeColorMode(colorMode);
        }
    }
    void clearAllCache();
    void drawPixmapSk(SkCanvas *canvas);
    void setDescendantCurrentGroup(const bool &bT);
    bool isDescendantCurrentGroup();
    bool shouldPaintOnImage();
    void drawUpdatePixmapSk(SkCanvas *canvas);

    virtual void addChildAwaitingUpdate(BoundingBox *child);
    void beforeUpdate();
    void processUpdate();
    void afterUpdate();
    void updateAfterCombinedTransformationChanged();
    void updateCombinedTransformTmp();
    void updateAfterCombinedTransformationChangedAfterFrameChagne();

    bool SWT_isBoxesGroup() { return true; }
protected:
    static bool mCtrlsAlwaysVisible;
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<QSharedPointer<BoundingBox> > mChildBoxes;

    QList<QSharedPointer<BoundingBox> > mChildrenAwaitingUpdate;
    QList<QSharedPointer<BoundingBox> > mUpdateChildrenAwaitingUpdate;
    int getChildBoxIndex(BoundingBox *child);
signals:
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
