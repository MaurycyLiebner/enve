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
    void requestContextMenu(QPoint);
private:
};

#endif // TRIGGERLABEL_H
