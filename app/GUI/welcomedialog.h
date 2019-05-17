#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H
#include <QDialog>
#include <functional>

class WelcomeDialog : public QDialog {
public:
    WelcomeDialog(const QStringList& recentPaths,
                  const std::function<void()>& newFunc,
                  const std::function<void()>& openFunc,
                  const std::function<void(QString)>& openRecentFunc,
                  QWidget * const parent = nullptr);
};

#endif // WELCOMEDIALOG_H
