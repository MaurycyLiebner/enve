#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"
#include "Boxes/vectorpath.h"
#include "fillstrokesettings.h"

#define foreachBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class MainWindow;

class Edge;

class BoxesGroup : public BoundingBox
{
    Q_OBJECT
public:
    BoxesGroup(BoxesGroup *parent);
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting);
    ~BoxesGroup();
    virtual void loadFromSql(int boundingBoxId);

    BoundingBox *createLink(BoxesGroup *parent);
    BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    void draw(QPainter *p);
//    void render(QPainter *p);
    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();

    BoundingBox *getBoxAt(QPointF absPos);
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);

    void addContainedBoxesToSelection(QRectF rect);

    void drawBoundingRect(QPainter *p);

    void setIsCurrentGroup(bool bT);

    BoundingBox *getPathAtFromAllAncestors(QPointF absPos);

    void setFillSettings(PaintSettings fillSettings,
                         bool saveUndoRedo = true);
    void setStrokeSettings(StrokeSettings strokeSettings,
                           bool saveUndoRedo = true);

    void setCurrentFillStrokeSettingsFromBox(BoundingBox *box);

    void ungroup();

    int saveToSql(QSqlQuery *query, int parentId);
    BoxesGroup *loadChildrenFromSql(int thisBoundingBoxId, bool loadInBox);

    //

    void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    void drawChildrenListItems(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    qreal getListItemHeight();
    void handleListItemMousePress(qreal boxesListX,
                                  qreal relX, qreal relY,
                                  QMouseEvent *event);
    void handleChildListItemMousePress(qreal boxesListX,
                                       qreal relX, qreal relY,
                                       qreal y0, QMouseEvent *event);

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

    void updateAfterCombinedTransformationChanged();
    void removeChildFromList(int id, bool saveUndoRedo = true);

    void updateAfterFrameChanged(int currentFrame);


    void setFillGradient(Gradient* gradient, bool finish);
    void setStrokeGradient(Gradient* gradient, bool finish);
    void setFillFlatColor(Color color, bool finish);
    void setStrokeFlatColor(Color color, bool finish);
    void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient);
    void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient);
    void setStrokeCapStyle(Qt::PenCapStyle capStyle);
    void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle);
    void setStrokeWidth(qreal strokeWidth, bool finish);

    void drawChildrenKeysView(QPainter *p,
                              qreal drawY, qreal maxY,
                              qreal pixelsPerFrame,
                              int startFrame, int endFrame);
    void drawKeysView(QPainter *p,
                      qreal drawY, qreal maxY,
                      qreal pixelsPerFrame,
                      int startFrame, int endFrame);

    PaintSettings *getFillSettings();
    StrokeSettings *getStrokeSettings();

    static bool getCtrlsAlwaysVisible();
    static void setCtrlsAlwaysVisible(bool bT);

    void updateAllBoxes();

    void updateBoundingRect();
    void applyCurrentTransformation();

    bool relPointInsidePath(QPointF relPos);
//    QPointF getRelCenterPosition();
    void updateEffectsMargin();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);
    SWT_Type SWT_getType() { return SWT_BoxesGroup; }

    bool SWT_satisfiesRule(const SWT_RulesCollection &rules,
                           const bool &parentSatisfies);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void makeDuplicate(BoundingBox *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    void applyPaintSetting(
            const PaintSetting &setting) {
        foreach(BoundingBox *box, mChildBoxes) {
            box->applyPaintSetting(setting);
        }
    }

    void setFillColorMode(const ColorMode &colorMode) {
        foreach(BoundingBox *box, mChildBoxes) {
            box->setFillColorMode(colorMode);
        }
    }
    void setStrokeColorMode(const ColorMode &colorMode) {
        foreach(BoundingBox *box, mChildBoxes) {
            box->setStrokeColorMode(colorMode);
        }
    }
    void clearCache();
protected:
    static bool mCtrlsAlwaysVisible;
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    QList<BoundingBox*> mChildBoxes;
signals:
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
