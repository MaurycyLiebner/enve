#ifndef TWOCOLUMNLAYOUT_H
#define TWOCOLUMNLAYOUT_H
#include <QHBoxLayout>
class TwoColumnLayout : public QHBoxLayout {
public:
    TwoColumnLayout();

    void addWidgetToFirstColumn(QWidget *wid);

    void addWidgetToSecondColumn(QWidget *wid);

    void addPair(QWidget *column1, QWidget *column2);
protected:
    QVBoxLayout *mLayout1 = nullptr;
    QVBoxLayout *mLayout2 = nullptr;
};

#endif // TWOCOLUMNLAYOUT_H
