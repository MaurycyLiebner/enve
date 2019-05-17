#include "welcomedialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QFontMetrics>

#include "global.h"
#include "BoxesList/OptimalScrollArea/scrollarea.h"

WelcomeDialog::WelcomeDialog(const QStringList &recentPaths,
                             const std::function<void()>& newFunc,
                             const std::function<void()>& openFunc,
                             const std::function<void(QString)>& openRecentFunc,
                             QWidget * const parent) :
    QDialog(parent) {
    setWindowFlag(Qt::FramelessWindowHint);

    const auto mainLay = new QVBoxLayout;
    setLayout(mainLay);

    const auto buttonLay = new QHBoxLayout;
    mainLay->addLayout(buttonLay);

    const auto newButton = new QPushButton("New", this);
    connect(newButton, &QPushButton::released, newFunc);
    buttonLay->addWidget(newButton);

    const auto openButton = new QPushButton("Open...", this);
    connect(openButton, &QPushButton::released, openFunc);
    buttonLay->addWidget(openButton);

    const auto recentScroll = new ScrollArea(this);

    const auto recentWidget = new QWidget(this);
    const auto recentLay = new QVBoxLayout;
    recentWidget->setLayout(recentLay);

    for(const auto& path : recentPaths) {
        const QFile file(path);
        if(!file.exists()) continue;
        QString cutPath;
        const auto fm = newButton->fontMetrics();
        int wholeWidth = fm.width(cutPath);
        const int dotsW = fm.width("...");
        bool addDots = false;
        while(wholeWidth > width() - 1.5*MIN_WIDGET_HEIGHT) {
            addDots = true;
            const int spaceForLetters = int(width() - 1.5*MIN_WIDGET_HEIGHT - dotsW);
            const int guessLen = spaceForLetters*cutPath.count()/wholeWidth;
            cutPath = cutPath.right(guessLen);
            wholeWidth = fm.width("..." + cutPath);
        }
        if(addDots) cutPath = "..." + cutPath;

        const auto pathButton = new QPushButton(cutPath, this);
        connect(pathButton, &QPushButton::released, [path, openRecentFunc]() {
            openRecentFunc(path);
        });
        recentLay->addWidget(pathButton);
    }

    recentScroll->setWidget(recentWidget);

    mainLay->addWidget(recentScroll);
}
