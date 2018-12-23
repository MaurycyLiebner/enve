#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "Boxes/pathbox.h"
#include "skia/skiaincludes.h"
class QStringAnimator;

class TextBox : public PathBox {
public:
    TextBox();

    void setFont(const QFont &font);
    void setSelectedFontSize(const qreal &size);
    void setSelectedFontFamilyAndStyle(const QString &fontFamily,
                                       const QString &fontStyle);

    qreal getFontSize();
    QString getFontFamily();
    QString getFontStyle();
    QString getCurrentTextValue();

    void openTextEditor(QWidget* dialogParent);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
//    QRectF getTextRect();
    void setPathText(bool pathText);

    void setTextAlignment(const Qt::Alignment &alignment) {
        mAlignment = alignment;
        scheduleUpdate(Animator::USER_CHANGE);
    }

    bool SWT_isTextBox() { return true; }
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction,
                                    QWidget* widgetsParent);
    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);
    void setCurrentTextValue(const QString &text,
                             const bool &saveUndoRedo = true);

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);

    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;
private:
    Qt::Alignment mAlignment = Qt::AlignLeft;
    QFont mFont;

    qsptr<QrealAnimator> mLinesDist;
    qsptr<QStringAnimator> mText;
};

#endif // TEXTBOX_H
