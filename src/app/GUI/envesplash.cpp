// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "envesplash.h"

#include <QPainter>

#include "Private/esettings.h"

EnveSplash::EnveSplash() {
    mTitle = "Welcome to Enve";
    mSubTitle = "open-source 2D animation software";
    mText = "Further development will only be possible with your support. \n"
            "Press 'Support enve' on the menu bar for more information.\n\n"
            "Please note that major version zero 0.x.y is for initial development; \n"
            "hence numerous bugs should be expected.";
    mSponsors = "Thank you for your support!";
    mAuthor = "Maurycy Liebner";

    const auto splashPath = eSettings::sIconsDir() + "/splash2021.png";
    const QPixmap pixmap(splashPath);
    const int x = qRound(0.4*pixmap.width());
    const int width = qRound(0.55*pixmap.width());

    mTitleRect = QRect(x, qRound(0.2*pixmap.height()),
                      width, qRound(0.15*pixmap.height()));

    mSubTitleRect = QRect(x, qRound(0.3*pixmap.height()),
                            width, qRound(0.1*pixmap.height()));

    mTextRect = QRect(x, qRound(0.4*pixmap.height()),
                      width, qRound(0.3*pixmap.height()));

    mSponsorsRect = QRect(x, qRound(0.65*pixmap.height()),
                        width, qRound(0.15*pixmap.height()));

    mAuthorRect = QRect(x, qRound(0.73*pixmap.height()),
                        width, qRound(0.1*pixmap.height()));

    mBottomRect = QRect(x, qRound(0.88*pixmap.height()),
                        width, qRound(0.09*pixmap.height()));
    setPixmap(pixmap);
    setFixedSize(pixmap.width(), pixmap.height());
//    setWindowFlag(Qt::WindowStaysOnTopHint);
}

void EnveSplash::drawContents(QPainter * const p) {
    p->setPen(Qt::white);

    QFont MainTitle = p->font();
    MainTitle.setPointSizeF(MainTitle.pointSizeF()*2.4);
    MainTitle.setWeight(QFont::Bold);
    p->setFont(MainTitle);
    p->drawText(mTitleRect,
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mTitle);

    QFont SubTitle = p->font();
    SubTitle.setPointSizeF(SubTitle.pointSizeF()*0.6);
    SubTitle.setWeight(QFont::Bold);
    p->setFont(SubTitle);
    p->drawText(mSubTitleRect.adjusted(4, 0, 0, 0),
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mSubTitle);

    QFont Texte = p->font();
    Texte.setPointSizeF(SubTitle.pointSizeF()*0.6);
    Texte.setWeight(QFont::Light);
    p->setFont(Texte);
    p->drawText(mTextRect.adjusted(4, 0, 0, 0),
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mText);


    QFont Sponsors = p->font();
    Sponsors.setPointSizeF(Sponsors.pointSizeF()*1.6666);
    Sponsors.setWeight(QFont::Bold);
    p->setFont(Sponsors);
    p->drawText(mSponsorsRect.adjusted(4, 0, 0, 0), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mSponsors);

    QFont Autor = p->font();
    Autor.setPointSizeF(Autor.pointSizeF()*0.6);
    Autor.setWeight(QFont::Light);
    p->setFont(Autor);
    p->drawText(mAuthorRect.adjusted(4, 0, 0, 0), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mAuthor);

    QString rightTxt;
   #if defined(LATEST_COMMIT_HASH) && defined(LATEST_COMMIT_DATE)
       const QString date(LATEST_COMMIT_DATE);
       rightTxt = QString(LATEST_COMMIT_HASH) + " " + date.split(" ").first();
   #else
       rightTxt = ENVE_VERSION;
   #endif
       p->drawText(mBottomRect, Qt::AlignVCenter | Qt::AlignRight, rightTxt);

}

void EnveSplash::mousePressEvent(QMouseEvent *) {
    if(parent()) deleteLater();
}
