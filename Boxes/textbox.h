#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "Boxes/pathbox.h"
#include "skiaincludes.h"

class TextBox : public PathBox
{
public:
    TextBox();

    void setText(const QString &text, const bool &saveUndoRedo = true);

    void setFont(const QFont &font, const bool &saveUndoRedo = true);
    void setSelectedFontSize(const qreal &size);
    void setSelectedFontFamilyAndStyle(const QString &fontFamily,
                                       const QString &fontStyle);

    qreal getFontSize();
    QString getFontFamily();
    QString getFontStyle();

    void openTextEditor(const bool &saveUndoRedo = true);
    int saveToSql(QSqlQuery *query, const int &parentId);
    void prp_loadFromSql(const int &boundingBoxId);
    void updatePath();
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
//    QRectF getTextRect();
    void setPathText(bool pathText);

    BoundingBox *createNewDuplicate() {
        return new TextBox();
    }

    void setTextAlignment(const Qt::Alignment &alignment) {
        mAlignment = alignment;
        scheduleSoftUpdate();
    }

    void makeDuplicate(Property *targetBox) {
        PathBox::makeDuplicate(targetBox);
        TextBox *textTarget = (TextBox*)targetBox;
        textTarget->setText(mText);
        textTarget->setFont(mFont);
        textTarget->setTextAlignment(mAlignment);
    }

    bool SWT_isTextBox() { return true; }
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction);

private:
    SkPath getPathAtRelFrame(const int &relFrame);

    QString mText;
    QFont mFont;
    Qt::Alignment mAlignment = Qt::AlignLeft;
};

#endif // TEXTBOX_H
