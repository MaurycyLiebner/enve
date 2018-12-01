#ifndef NOSHORTCUTACTION_H
#define NOSHORTCUTACTION_H
#include <QAction>

class NoShortcutAction : public QAction
{
public:
    NoShortcutAction(const QString &text,
                     const QObject *receiver,
                     const char *member,
                     const QKeySequence &shortcut = 0,
                     QObject *parent = nullptr);

protected:
    bool event(QEvent *e);
};

#endif // NOSHORTCUTACTION_H
