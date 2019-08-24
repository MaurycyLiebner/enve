// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#include "usagewidget.h"
#include <QTimer>
#include <QLocale>
#include <QHBoxLayout>
#include "GUI/global.h"
#include "Tasks/taskscheduler.h"
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
    int mValueSum = 0;
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

    connect(TaskScheduler::sInstance, &TaskScheduler::hddUsageChanged,
            this, &UsageWidget::setHddUsage);
    connect(TaskScheduler::sInstance, &TaskScheduler::gpuUsageChanged,
            this, &UsageWidget::setGpuUsage);
    connect(TaskScheduler::sInstance, &TaskScheduler::cpuUsageChanged,
            this, &UsageWidget::setThreadsUsage);
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
