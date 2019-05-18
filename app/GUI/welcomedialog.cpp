#include "welcomedialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFontMetrics>
#include <QDir>

#include "global.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"

WelcomeDialog::WelcomeDialog(const QStringList &recentPaths,
                             const std::function<void()>& newFunc,
                             const std::function<void()>& openFunc,
                             const std::function<void(QString)>& openRecentFunc,
                             QWidget * const parent) :
    QWidget(parent) {
    const auto thisLay = new QVBoxLayout;

    const auto mainWid = new QWidget(this);
    const int size = MIN_WIDGET_HEIGHT*12;
    mainWid->setMinimumWidth(size);
    setLayout(thisLay);
    thisLay->addWidget(mainWid, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    const auto mainLay = new QVBoxLayout;
    mainWid->setLayout(mainLay);

    const auto buttonLay = new QHBoxLayout;
    mainLay->addLayout(buttonLay);

    const auto newButton = new QPushButton("New", this);
    connect(newButton, &QPushButton::released, newFunc);
    buttonLay->addWidget(newButton);

    const auto openButton = new QPushButton("Open...", this);
    connect(openButton, &QPushButton::released, openFunc);
    buttonLay->addWidget(openButton);

    if(recentPaths.isEmpty()) return;
    const auto recentWidget = new QWidget(this);
    const auto recentLay = new QVBoxLayout;
    recentLay->setSpacing(0);
    recentLay->setAlignment(Qt::AlignTop);
    recentLay->setContentsMargins(0, 0, 0, 0);

    recentWidget->setLayout(recentLay);

    int i = 0;
    const int buttSize = size - 3*MIN_WIDGET_HEIGHT;
    for(const auto& path : recentPaths) {
        QString cutPath = path;
        if(true) cutPath = cutPath.split("/").last();
        const auto fm = newButton->fontMetrics();
        int wholeWidth = fm.width(cutPath);
        const int dotsW = fm.width("...");
        bool addDots = false;
        while(wholeWidth > buttSize) {
            addDots = true;
            const int spaceForLetters = int(buttSize - dotsW);
            const int guessLen = spaceForLetters*cutPath.count()/wholeWidth;
            cutPath = cutPath.right(guessLen);
            wholeWidth = fm.width("..." + cutPath);
        }
        if(addDots) cutPath = "..." + cutPath;

        const auto pathButton = new QPushButton(cutPath, this);
        pathButton->setMinimumHeight(5*MIN_WIDGET_HEIGHT/4);
        const auto homePath = QDir::homePath();
        QString ttPath = path;
        if(ttPath.contains(homePath)) {
            ttPath = "~" + ttPath.mid(homePath.count());
        }
        pathButton->setToolTip("<p style='white-space:pre'>" + ttPath + "</p>");
        connect(pathButton, &QPushButton::released, [path, openRecentFunc]() {
            openRecentFunc(path);
        });
        recentLay->addWidget(pathButton, 0, Qt::AlignTop);
        i++;
        if(i == 1 && i == recentPaths.count()) {
        } else if(i == 1) {
            pathButton->setStyleSheet("QPushButton {"
                "border-bottom-left-radius: 0;"
                "border-bottom-right-radius: 0;"
                "border-bottom: 0;"
            "}");
        } else if(i == recentPaths.count() || i == 8) {
            pathButton->setStyleSheet("QPushButton {"
                "border-top-left-radius: 0;"
                "border-top-right-radius: 0;"
            "}");
            break;
        } else {
            pathButton->setStyleSheet("QPushButton {"
                "border-radius: 0;"
                "border-bottom: 0;"
            "}");
        }
    }

    mainLay->addSpacing(MIN_WIDGET_HEIGHT);
//    const auto recentScroll = new ScrollArea(this);
//    recentScroll->setWidget(recentWidget);
//    mainLay->addWidget(recentScroll);
    mainLay->addWidget(recentWidget);
}
#include <QPainter>
void WelcomeDialog::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), Qt::black);
    p.end();
}
