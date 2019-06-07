#ifndef UNIFORMSPECIFIERCREATOR_H
#define UNIFORMSPECIFIERCREATOR_H
#include "Animators/qrealanimatorcreator.h"
#include "Animators/intanimatorcreator.h"
#include <QJSEngine>

qreal stringToDouble(const QString& str);
int stringToInt(const QString& str);

typedef std::function<void(QGL33c * const, QJSEngine&)> UniformSpecifier;
typedef QList<UniformSpecifier> UniformSpecifiers;
struct UniformSpecifierCreator : public StdSelfRef {
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal relFrame) const = 0;
};

struct QrealAnimatorUniformSpecifierCreator :
        public UniformSpecifierCreator {
    QrealAnimatorUniformSpecifierCreator(const QString& script) :
        mScript(script) {}
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal relFrame) const;

    static void sTestScript(const QString& script, const QString& propName);
private:
    QString mScript;
};

struct IntAnimatorUniformSpecifierCreator :
        public UniformSpecifierCreator {
    IntAnimatorUniformSpecifierCreator(const QString& script) :
        mScript(script) {}
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal relFrame) const;

    static void sTestScript(const QString& script, const QString& propName);
private:
    QString mScript;
};
#endif // UNIFORMSPECIFIERCREATOR_H
