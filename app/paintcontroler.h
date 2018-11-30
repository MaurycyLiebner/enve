#ifndef PAINTCONTROLER_H
#define PAINTCONTROLER_H
#include <QObject>
#include <QTimer>
#include "Boxes/boundingbox.h"

class PaintControler : public QObject {
    Q_OBJECT
public:
    explicit PaintControler(const int &id,
                            QObject *parent = 0);

signals:
    void finishedUpdating(int, _Executor*);
public slots:
    void updateUpdatable(_Executor *updatable,
                         const int &targetId);
private:
    int mId;
};

#endif // PAINTCONTROLER_H
