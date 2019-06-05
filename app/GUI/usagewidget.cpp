#include "usagewidget.h"
#include <QTimer>
#include <QLocale>

UsageWidget::UsageWidget(QWidget *parent) : QLabel(parent) {
    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    setThreadsTotal(QThread::idealThreadCount());
}

void UsageWidget::setThreadsUsage(const int threads) {
    if(threads >= 0) {
        mThreadsUsage = threads;
    } else {
        mThreadsUsage = mThreadsTotal + threads;
    }
    updateDisplayedText();
}

void UsageWidget::setThreadsTotal(const int threads) {
    mThreadsTotal = threads;
    updateDisplayedText();
}

void UsageWidget::setHddUsage(const bool used) {
    mHddUsage = used;
    updateDisplayedText();
}

void UsageWidget::setRamUsage(const qreal thisGB) {
    if(thisGB >= 0) {
        mRamUsage = thisGB;
    } else {
        mRamUsage = mTotalRam + thisGB;
    }
    updateDisplayedText();
}

void UsageWidget::setTotalRam(const qreal totalRamGB) {
    mTotalRam = totalRamGB;
    updateDisplayedText();
}

void UsageWidget::updateDisplayedText() {
    QLocale locale;
    const int cpuPer = qRound(mThreadsUsage*100./mThreadsTotal);
    const int hddPer = mHddUsage ? 100 : 0;
    const int ramPer = qRound(mRamUsage*100./mTotalRam);
    setText("cpu: " + locale.toString(cpuPer) + "%     " +
            "hdd: " + locale.toString(hddPer) + "%     " +
            "ram: " + locale.toString(ramPer) + "%");
//            locale.toString(mRamUsage, 'f', 1) + " GB / " +
//            locale.toString(mTotalRam, 'f', 1) + " GB");
}
