#ifndef PAINTCONTROLER_H
#define PAINTCONTROLER_H
#include <QObject>
#include <QTimer>
#include "Boxes/boundingbox.h"

class PaintControler : public QObject
{
    Q_OBJECT
public:
    explicit PaintControler(QObject *parent = 0);

signals:
    void finishedUpdatingLastBox();
public slots:
    void updateBoxPixmaps(BoundingBox *box);
private:
};

#endif // PAINTCONTROLER_H
