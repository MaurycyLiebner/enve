#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "Boxes/pathbox.h"
#include "skia/skiaincludes.h"
class QStringAnimator;

class TextBox : public PathBox {
public:
    TextBox();

    bool SWT_isTextBox() const { return true; }
    void addActionsToMenu(BoxTypeMenu * const menu);
    SkPath getPathAtRelFrameF(const qreal &relFrame);

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;

    void setFont(const QFont &font);
    void setSelectedFontSize(const qreal &size);
    void setSelectedFontFamilyAndStyle(const QString &fontFamily,
                                       const QString &fontStyle);

    qreal getFontSize();
    QString getFontFamily();
    QString getFontStyle();
    QString getCurrentValue();

    void openTextEditor(QWidget* dialogParent);
//    QRectF getTextRect();
    void setPathText(bool pathText);

    void setTextAlignment(const Qt::Alignment &alignment) {
        mAlignment = alignment;
        planScheduleUpdate(Animator::USER_CHANGE);
    }

    void setCurrentValue(const QString &text);
private:
    Qt::Alignment mAlignment = Qt::AlignLeft;
    QFont mFont;

    qsptr<QrealAnimator> mLinesDist;
    qsptr<QStringAnimator> mText;
};

#endif // TEXTBOX_H
