#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H
#include <QWidget>
#include <functional>

class WelcomeDialog : public QWidget {
public:
    WelcomeDialog(const QStringList& recentPaths,
                  const std::function<void()>& newFunc,
                  const std::function<void()>& openFunc,
                  const std::function<void(QString)>& openRecentFunc,
                  QWidget * const parent = nullptr);
protected:
    void paintEvent(QPaintEvent *);
};

#endif // WELCOMEDIALOG_H
