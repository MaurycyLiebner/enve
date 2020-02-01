#ifndef EXPRESSIONSOURCE_H
#define EXPRESSIONSOURCE_H
#include "expressionsourcebase.h"

class ExpressionSource : public ExpressionSourceBase {
public:
    ExpressionSource(QrealAnimator* const parent);

    static sptr sCreate(const QString& path, QrealAnimator* const parent);

    qreal calculateValue(const qreal relFrame) const override;
    FrameRange identicalRange(const qreal relFrame) const override;
    QString toString() const override
    { return mPath; }

    void setPath(const QString& path);
private:
    void lookForSource();
    void updateSourcePath();

    QString mPath;
};

#endif // EXPRESSIONSOURCE_H
