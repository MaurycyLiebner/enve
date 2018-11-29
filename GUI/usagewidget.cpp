#include "usagewidget.h"
#include <QTimer>
#include <QLocale>

UsageWidget::UsageWidget(QWidget *parent) : QLabel(parent) {
    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    setThreadsTotal(QThread::idealThreadCount());
}

void UsageWidget::setThreadsUsage(const int &threads) {
    if(threads >= 0) {
        mThreadsUsage = threads;
    } else {
        mThreadsUsage = mThreadsTotal + threads;
    }
    updateDisplayedText();
}

void UsageWidget::setThreadsTotal(const int &threads) {
    mThreadsTotal = threads;
    updateDisplayedText();
}

void UsageWidget::setCpuUsage(const int &thisPercent) {
    mCpuUsage = thisPercent;
    updateDisplayedText();
}

void UsageWidget::setRamUsage(const qreal &thisGB) {
    if(thisGB >= 0.) {
        mRamUsage = thisGB;
    } else {
        mRamUsage = mTotalRam + thisGB;
    }
    updateDisplayedText();
}

void UsageWidget::setTotalRam(const qreal &totalRamGB) {
    mTotalRam = totalRamGB;
    updateDisplayedText();
}

void UsageWidget::updateDisplayedText() {
    QLocale locale;
#ifdef QT_DEBUG
    setText("threads: " + locale.toString(mThreadsUsage) + " / " +
            locale.toString(mThreadsTotal) + "     " + "cpu: " +
            locale.toString(mCpuUsage) + "%" + "     " + "ram: " +
            locale.toString(mRamUsage, 'f', 1) + " GB / " +
            locale.toString(mTotalRam, 'f', 1) + " GB");
#else
    setText(locale.toString(mRamUsage, 'f', 1) + " GB / " +
            locale.toString(mTotalRam, 'f', 1) + " GB");
#endif
}
