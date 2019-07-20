#ifndef USAGEWIDGET_H
#define USAGEWIDGET_H

#include <QLabel>
#include <QThread>
#include <QStatusBar>

class HardwareUsageWidget;

class UsageWidget : public QStatusBar {
public:
    explicit UsageWidget(QWidget * const parent = nullptr);
    void setThreadsUsage(const int threads);
    void setThreadsTotal(const int threads);
    void setHddUsage(const bool used);
    void setGpuUsage(const bool used);
    void setRamUsage(const qreal thisGB);
    void setTotalRam(const qreal totalRamGB);
private:
    HardwareUsageWidget* mGpuBar;
    HardwareUsageWidget* mCpuBar;
    HardwareUsageWidget* mHddBar;
    HardwareUsageWidget* mRamBar;
    QLabel* mRamLabel;
};

#endif // USAGEWIDGET_H
