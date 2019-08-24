#include "envesplash.h"

#include <QPainter>

#include "esettings.h"

EnveSplash::EnveSplash() {
    mText = "Welcome to enve - an opensource 2D animations software.\n\n"
            "Further development will not be possible without your support. "
            "Press 'Support' in menu bar to enter $3500 crowdfunding campaign for next "
            "6 months of full-time development.\n\n"
            "Please note that the following executable is merely "
            "a snapshot of the current state of the code. "
            "Hence numerous bugs should be expected.\n\n"
            "Remember - enve is not a video editor.";
    const auto splashPath = eSettings::sIconsDir() + "/splash.png";
    const QPixmap pixmap(splashPath);
    mTextRect = QRect(qRound(0.03*pixmap.width()), qRound(0.20*pixmap.height()),
                      qRound(0.94*pixmap.width()), qRound(0.58*pixmap.height()));
    mMessageRect = QRect(mTextRect.x(), qRound(0.82*pixmap.height()),
                         qRound(0.94*pixmap.width()), qRound(0.09*pixmap.height()));
    setPixmap(pixmap);
    setFixedSize(pixmap.width(), pixmap.height());
    setWindowFlag(Qt::WindowStaysOnTopHint);
}

void EnveSplash::drawContents(QPainter * const p) {
#ifdef QT_DEBUG
//    p->setPen(QColor(125, 125, 125));
//    p->drawRect(mTextRect);
//    p->drawRect(mMessageRect);
#endif
    p->setPen(Qt::white);

    QFont font = p->font();
    font.setPointSizeF(font.pointSizeF()*1.5);
    font.setFamily("FreeMono");
    p->setFont(font);
    const int w = width();
    const int marg = w/80;
    p->drawText(mTextRect.adjusted(marg, marg, -marg, -marg),
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mText);
    p->drawText(mMessageRect, Qt::AlignCenter, message());
}

void EnveSplash::mousePressEvent(QMouseEvent *) {
    if(parent()) deleteLater();
}
