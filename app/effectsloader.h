#ifndef EFFECTSLOADER_H
#define EFFECTSLOADER_H
#include "offscreenqgl33c.h"

class ShaderEffectProgram;
class ShaderEffectCreator;

class EffectsLoader : public QObject, protected OffscreenQGL33c {
    Q_OBJECT
public:
    EffectsLoader();

    void initialize();
signals:
    void programChanged(ShaderEffectProgram*);
private:
    void iniRasterEffectPrograms();
    void reloadProgram(ShaderEffectCreator * const loaded,
                       const QString& fragPath);
    void iniSingleRasterEffectProgram(const QString &grePath);
    void iniRasterEffectProgramExec(const QString &grePath);

    void iniCustomPathEffects();

    QStringList mLoadedGREPaths;
    GLuint mPlainSquareVAO;
    GLuint mTexturedSquareVAO;
};

#endif // EFFECTSLOADER_H
