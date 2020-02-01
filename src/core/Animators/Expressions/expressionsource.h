#ifndef EXPRESSIONSOURCE_H
#define EXPRESSIONSOURCE_H
#include "expressionvalue.h"
#include "conncontextptr.h"

class QrealAnimator;

class ExpressionSource : public ExpressionValue {
public:
    ExpressionSource(QrealAnimator* const parent);

    static sptr sCreate(const QString& path, QrealAnimator* const parent);

    qreal calculateValue(const qreal relFrame) const override;
    void collapse() override {}
    bool isPlainValue() const override
    { return false; }
    bool isValid() const override;
    FrameRange identicalRange(const qreal relFrame) const override;

    void setPath(const QString& path);
    void lookForSource();
private:
    QPointer<QrealAnimator> mParent;
    ConnContextQPtr<QrealAnimator> mSource;
    QStringList mPath;
};

#endif // EXPRESSIONSOURCE_H
