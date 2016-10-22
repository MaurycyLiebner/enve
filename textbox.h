#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "pathbox.h"

class TextBox : public PathBox
{
public:
    TextBox(BoxesGroup *parent);
    //void draw(QPainter *p);

    //QRectF getBoundingRect();

    void setText(QString text);

    void setFont(QFont font);
    void setFontSize(qreal size);
    void setFontFamilyAndStyle(QString fontFamily,
                               QString fontStyle);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    void openTextEditor();
    int saveToSql(int parentId);
    void loadFromSql(int boundingBoxId);
    void updatePath();
    MovablePoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);
    void schedulePathUpdateIfPathText();
    void draw(QPainter *p);
    QRectF getTextRect();
    bool pointInsidePath(QPointF point);
    void setPathText(bool pathText);
    QRectF getBoundingRect();
    void centerPivotPosition();
private:
    bool mPathText = false;
    QString mText;
    QFont mFont;
    Qt::Alignment mAlignment = Qt::AlignLeft;
};

#endif // TEXTBOX_H
