#ifndef EKEYFILTER_H
#define EKEYFILTER_H

#include <QObject>
#include <functional>

class MainWindow;

class eKeyFilter : public QObject {
public:
    explicit eKeyFilter(MainWindow * const window);

    static eKeyFilter* sCreateLineFilter(MainWindow * const window);
    static eKeyFilter* sCreateNumberFilter(MainWindow * const window);
protected:
    std::function<bool(int)> mAllow;

    bool eventFilter(QObject *watched, QEvent *event);
private:
    MainWindow * const mMainWindow;
};

#endif // EKEYFILTER_H
