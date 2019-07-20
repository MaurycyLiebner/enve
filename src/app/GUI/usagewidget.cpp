#include "usagewidget.h"
#include <QTimer>
#include <QLocale>
#include <QHBoxLayout>
#include "global.h"
#include <QProgressBar>

class HardwareUsageWidget : public QProgressBar {
public:
    HardwareUsageWidget(QWidget* const parent = nullptr) :
        QProgressBar(parent) {
        setFixedHeight(MIN_WIDGET_DIM/2);
        setObjectName("hardwareUsage");
        setSizePolicy(QSizePolicy::Maximum,
                      QSizePolicy::Maximum);
    }

    void pushValue(const int value) {
        if(mValues.count() > 3) popValue();
        mValues << value;
        mValueSum += value;
        updateValue();
    }

    void popValue() {
        mValueSum -= mValues.takeFirst();
        updateValue();
    }

    void popAllButLast() {
        if(mValues.isEmpty()) return;
        const int lastValue = mValues.last();
        mValues.clear();
        mValueSum = 0;
        pushValue(lastValue);
    }
private:
    void updateValue() {
        if(mValues.isEmpty()) return setValue(0);
        setValue(mValueSum/mValues.count());
    }

    QList<int> mValues;
    int mValueSum;
};

UsageWidget::UsageWidget(QWidget * const parent) : QStatusBar(parent) {
    setContentsMargins(0, 0, 0, 0);
    const auto layout = QStatusBar::layout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);

    const auto gpuLabel = new QLabel("  gpu: ", this);
    mGpuBar = new HardwareUsageWidget(this);
    mGpuBar->setRange(0, 100);

    const auto cpuLabel = new QLabel("  cpu: ", this);
    mCpuBar = new HardwareUsageWidget(this);

    const auto hddLabel = new QLabel("  hdd: ", this);
    mHddBar = new HardwareUsageWidget(this);
    mHddBar->setRange(0, 100);

    const auto ramLabel = new QLabel("  ram: ", this);
    mRamBar = new HardwareUsageWidget(this);

    mRamLabel = new QLabel(this);

    addPermanentWidget(gpuLabel);
    addPermanentWidget(mGpuBar);

    addPermanentWidget(cpuLabel);
    addPermanentWidget(mCpuBar);

    addPermanentWidget(hddLabel);
    addPermanentWidget(mHddBar);

    addPermanentWidget(ramLabel);
    addPermanentWidget(mRamBar);

    addPermanentWidget(mRamLabel);

    setThreadsTotal(QThread::idealThreadCount());
}

void UsageWidget::setThreadsUsage(const int threads) {
    mCpuBar->pushValue(threads);
}

void UsageWidget::setThreadsTotal(const int threads) {
    mCpuBar->setRange(0, threads);
}

void UsageWidget::setGpuUsage(const bool used) {
    mGpuBar->pushValue(used ? 100 : 0);
}

void UsageWidget::setHddUsage(const bool used) {
    mHddBar->pushValue(used ? 100 : 0);
}

void UsageWidget::setRamUsage(const qreal thisGB) {
    mRamBar->setValue(qRound(thisGB*1000));
    mGpuBar->popAllButLast();
    mCpuBar->popAllButLast();
    mHddBar->popAllButLast();
}

void UsageWidget::setTotalRam(const qreal totalRamGB) {
    mRamBar->setRange(0, qRound(totalRamGB*1000));
}
