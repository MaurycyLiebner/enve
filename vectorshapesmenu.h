#ifndef VECTORSHAPESMENU_H
#define VECTORSHAPESMENU_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class BoundingBox;

class VectorPath;

class VectorPathShape;

class VectorShapesMenu;

class ShapeWidget : public QWidget
{
    Q_OBJECT
public:
    ShapeWidget(VectorPathShape *shape, VectorShapesMenu *parent = 0);

    void setSelected(bool bT);
    void mousePressEvent(QMouseEvent *event);
    VectorPathShape *getShape();
private:
    VectorPathShape *mParentShape;
    bool mSelected = false;
    QHBoxLayout *mLayout;
    VectorShapesMenu *mParentMenu;
};

class VectorShapesMenu : public QWidget
{
    Q_OBJECT
public:
    explicit VectorShapesMenu(QWidget *parent = 0);

    void setSelectedBoundingBox(BoundingBox *box);
    void clearShapes();
    void addShapeWidgetForShape(VectorPathShape *shape);

    void setSelected(ShapeWidget *widget);
signals:

public slots:
protected:
    void setAllButEditButtonEnabled(bool enabled);
protected slots:
    void createNewShape();
    void removeCurrentShape();
    void editCurrrentShape();
    void finishEditingCurrrentShape();
    void cancelEditCurrentShape();
private:
    bool mEditingShape = false;

    ShapeWidget *createNewWidgetForShape(VectorPathShape *shape);

    VectorPath *mCurrentVectorPath = NULL;

    QScrollArea  *mScrollArea;
    QWidget *mScrollWidget;
    QVBoxLayout *mScrollLayout;

    QVBoxLayout *mMainLayout;
    QHBoxLayout *mButtonsLayout;

    QPushButton *mNewShapeButton;
    QPushButton *mRemoveShapeButton;
    QPushButton *mEditShapeButton;
    QPushButton *mCancelEditButton;

    QList<ShapeWidget*> mShapeWidgets;

    ShapeWidget *mSelectedShapeWidget = NULL;
};

#endif // VECTORSHAPESMENU_H
