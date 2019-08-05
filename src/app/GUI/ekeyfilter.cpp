#include "ekeyfilter.h"
#include "mainwindow.h"
#include "keyfocustarget.h"

eKeyFilter::eKeyFilter(MainWindow * const window) :
    QObject(window), mMainWindow(window) {}

eKeyFilter *eKeyFilter::sCreateLineFilter(MainWindow * const window) {
    const auto filter = new eKeyFilter(window);
    filter->mAllow = [](const int key) {
        for(int i = Qt::Key_F1; i <= Qt::Key_F12; i++) {
            if(key == i) return false;
        }
        if(key == Qt::Key_Return) {
            KeyFocusTarget::KFT_sSetRandomTarget();
            return false;
        }
        return true;
    };
    return filter;
}

eKeyFilter *eKeyFilter::sCreateNumberFilter(MainWindow * const window) {
    const auto filter = new eKeyFilter(window);
    filter->mAllow = [](const int key) {
        for(int i = Qt::Key_F1; i <= Qt::Key_F12; i++) {
            if(key == i) return false;
        }
        for(int i = Qt::Key_A; i <= Qt::Key_Z; i++) {
            if(key == i) return false;
        }
        if(key == Qt::Key_Return) {
            KeyFocusTarget::KFT_sSetRandomTarget();
            return false;
        }
        return true;
    };
    return filter;
}

bool eKeyFilter::eventFilter(QObject *watched, QEvent *event) {
    Q_UNUSED(watched);
    if(event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease) {
        const auto kEvent = static_cast<QKeyEvent*>(event);
        if(mAllow(kEvent->key())) return false;
        if(mMainWindow->processKeyEvent(kEvent)) return true;
        return false;
    }
    return false;
}
