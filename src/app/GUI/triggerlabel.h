#ifndef TRIGGERLABEL_H
#define TRIGGERLABEL_H
#include <QLabel>

class TriggerLabel : public QLabel {
    Q_OBJECT
public:
    using QLabel::QLabel;
protected:
    void mousePressEvent(QMouseEvent *ev);
signals:
    void triggered();
private:
};

#endif // TRIGGERLABEL_H
