#ifndef RENDEROUTPUTWIDGET_H
#define RENDEROUTPUTWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class RenderOutputWidget : public QDialog
{
    Q_OBJECT
public:
    explicit RenderOutputWidget(QWidget *parent = 0);

private:
    QHBoxLayout *mPathLayout;
    QLabel *mPathLabel;
    QPushButton *mSelectPathButton;

    QVBoxLayout *mMainLayout;

    QPushButton *mRenderButton;
signals:
    void render(QString dest);
public slots:
    void emitRender();
    void chooseDir();
};

#endif // RENDEROUTPUTWIDGET_H
