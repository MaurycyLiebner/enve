#ifndef BRUSHSELECTIONWIDGET_H
#define BRUSHSELECTIONWIDGET_H

#include <QWidget>
#include <QGridLayout>

class Brush;

class BrushSelectionWidget : public QWidget {
    Q_OBJECT
public:
    explicit BrushSelectionWidget(QWidget *parent = nullptr);
    const Brush *getCurrentBrush() const;
    void duplicateCurrentBrush();
    void saveBrushesForProject(QIODevice *target);
    void readBrushesForProject(QIODevice *target);
protected:
    Brush *getBrushAt(const QPoint &pos);
    int maxNumberColumns() const;
    int maxNumberRows() const;
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *);
    void addBrush(Brush *brush);
    void removeCurrentBrush();
signals:
    void brushSelected(const Brush*);
    void brushReplaced(const Brush*, const Brush*); // second one is new
public slots:
    void setCurrentBrush(const Brush *brush);
private:
    void removeBrush(Brush *brush);
    int mNumberColumns = 0;
    int mDimension = 48;
    int mMargin = 4;
    QList<Brush*> mAllBrushes;
    Brush *mDefaultBrush = NULL;
    Brush *mCurrentBrush = NULL;
};

#endif // BRUSHSELECTIONWIDGET_H
