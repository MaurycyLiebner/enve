#ifndef NOSHORTCUTACTION_H
#define NOSHORTCUTACTION_H
#include <QAction>

class NoShortcutAction : public QAction {
public:
    template <typename T1, typename T2>
    NoShortcutAction(const QString &text,
                     const T1 * const receiver,
                     const T2 &member,
                     const QKeySequence &shortcut = 0,
                     QObject * const parent = nullptr) :
        QAction(text, parent) {
        connect(this, &QAction::triggered, receiver, member);
        setShortcut(shortcut);
    }

protected:
    bool event(QEvent *e);
};

#endif // NOSHORTCUTACTION_H
