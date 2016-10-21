#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "boundingbox.h"

class TextBox : public BoundingBox
{
public:
    TextBox(BoxesGroup *parent);
    void draw(QPainter *p);

    QRectF getBoundingRect();

    void setText(QString text);

    void setCurrentText(QString text);
    void setFont(QFont font);
    void setFontSize(qreal size);
    void setFontFamilyAndStyle(QString fontFamily,
                               QString fontStyle);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
    bool pointInsidePath(QPointF point);
    void centerPivotPosition();
    QRectF getTextRect();

    void openTextEditor();
private:
    QString mText;
    QFont mFont;
    Qt::Alignment mAlignment = Qt::AlignLeft;
};

#endif // TEXTBOX_H
