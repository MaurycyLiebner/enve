#ifndef EFFECTSLOADER_H
#define EFFECTSLOADER_H
#include "Tasks/offscreenqgl33c.h"

class ShaderEffectProgram;
class ShaderEffectCreator;

class EffectsLoader : public QObject, protected OffscreenQGL33c {
    Q_OBJECT
public:
    EffectsLoader();
    ~EffectsLoader();

    void initializeGpu();
    void iniCustomPathEffects();
    void iniCustomRasterEffects();
    void iniShaderEffects();
    void iniCustomBoxes();
signals:
    void programChanged(ShaderEffectProgram*);
private:
    void reloadProgram(ShaderEffectCreator * const loaded,
                       const QString& fragPath);
    void iniSingleRasterEffectProgram(const QString &grePath);
    void iniShaderEffectProgramExec(const QString &grePath);

    void iniCustomRasterEffect(const QString &soPath);
    void iniIfCustomRasterEffect(const QString &path);

    QStringList mLoadedGREPaths;
    GLuint mPlainSquareVAO;
    GLuint mTexturedSquareVAO;
};

#endif // EFFECTSLOADER_H
