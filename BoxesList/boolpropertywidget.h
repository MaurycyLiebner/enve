#ifndef BOOLPROPERTYWIDGET_H
#define BOOLPROPERTYWIDGET_H

#include <QCheckBox>
class BoolProperty;
class BoolPropertyContainer;

class BoolPropertyWidget : public QWidget {
    Q_OBJECT
public:
    BoolPropertyWidget(QWidget *parent = 0);

    void setTarget(BoolProperty *property);
    void setTarget(BoolPropertyContainer *property);
protected:
    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
private:
    bool mHovered = false;
    BoolProperty *mTarget = nullptr;
    BoolPropertyContainer *mTargetContainer = nullptr;
};

#endif // BOOLPROPERTYWIDGET_H
