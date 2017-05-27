#include "canvaswidget.h"
#include "canvaswindow.h"

#include <QMdiSubWindow>
#include <QAbstractScrollArea>
#include <QtWidgets/private/qwidget_p.h>

class QWindowContainerPrivate : public QWidgetPrivate
{
public:
    Q_DECLARE_PUBLIC(QWindowContainer)

    QWindowContainerPrivate()
        : window(0)
        , oldFocusWindow(0)
        , usesNativeWidgets(false)
    {
    }

    ~QWindowContainerPrivate() { }

    static QWindowContainerPrivate *get(QWidget *w) {
        QWindowContainer *wc = qobject_cast<QWindowContainer *>(w);
        if (wc)
            return wc->d_func();
        return 0;
    }

    void updateGeometry() {
        Q_Q(QWindowContainer);
        if (usesNativeWidgets)
            window->setGeometry(q->rect());
        else
            window->setGeometry(QRect(q->mapTo(q->window(), QPoint()), q->size()));
    }

    void updateUsesNativeWidgets()
    {
        if (usesNativeWidgets || window->parent() == 0)
            return;
        Q_Q(QWindowContainer);
        QWidget *p = q->parentWidget();
        while (p) {
            if (qobject_cast<QMdiSubWindow *>(p) != 0
                    || qobject_cast<QAbstractScrollArea *>(p) != 0) {
                q->winId();
                usesNativeWidgets = true;
                break;
            }
            p = p->parentWidget();
        }
    }

    void markParentChain() {
        Q_Q(QWindowContainer);
        QWidget *p = q;
        while (p) {
            QWidgetPrivate *d = static_cast<QWidgetPrivate *>(QWidgetPrivate::get(p));
            d->createExtra();
            d->extra->hasWindowContainer = true;
            p = p->parentWidget();
        }
    }

    bool isStillAnOrphan() const {
        return window->parent() == &fakeParent;
    }

    QPointer<QWindow> window;
    QWindow *oldFocusWindow;
    QWindow fakeParent;

    uint usesNativeWidgets : 1;
};


CanvasWidget::CanvasWidget(CanvasWindow *window,
                           QWidget *parent,
                           const Qt::WindowFlags &flags) :
    QWindowContainer(window, parent, flags) {
    mCanvasWindow = window;
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);
}

void CanvasWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);

    p.fillRect(0, 0, 100, 100, Qt::red);
}
