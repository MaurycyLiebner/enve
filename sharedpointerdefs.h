#ifndef SHAREDPOINTERDEFS_H
#define SHAREDPOINTERDEFS_H

#include <QSharedPointer>
#include <QPointer>
#include <memory>
#include "stdpointer.h"

#define getAsPtr(baseObjT, derivedClassT) derivedClassT::getAsPtr<derivedClassT>(baseObjT)
#define getAsSPtr(baseObjT, derivedClassT) derivedClassT::getAsSPtr<derivedClassT>(baseObjT)

#define SPtrCreate(classT) classT::create<classT>
#define classQPtrs(classT) class classT; \
    typedef QSharedPointer<classT> classT##QSPtr; \
    typedef QPointer<classT> classT##QPtr;
#define classPtrs(classT) class classT; \
    typedef std::shared_ptr<classT> classT##SPtr; \
    typedef StdPointer<classT> classT##Ptr;
#define structPtrs(structT) struct structT; \
    typedef std::shared_ptr<structT> structT##SPtr; \
    typedef StdPointer<structT> structT##Ptr;

classQPtrs(BoundingBox)
classQPtrs(BoxesGroup)
classQPtrs(Canvas)
classQPtrs(PathBox)
classQPtrs(VectorPath)
classQPtrs(Circle)
classQPtrs(Rectangle)
classQPtrs(TextBox)
classQPtrs(ParticleBox)
classQPtrs(ImageBox)
classQPtrs(ImageSequenceBox)
classQPtrs(VideoBox)
classQPtrs(PaintBox)
classQPtrs(BonesBox)
classQPtrs(ExternalLinkBox)
classQPtrs(InternalLinkCanvas)

classQPtrs(SingleWidgetTarget)
classQPtrs(WindowSingleWidgetTarget)

classQPtrs(Property)
classQPtrs(IntProperty)
classQPtrs(BoolProperty)
classQPtrs(ComboBoxProperty)

classQPtrs(Animator)
classQPtrs(QStringAnimator)
classQPtrs(QPointFAnimator)
classQPtrs(QrealAnimator)
classQPtrs(RandomQrealGenerator)
classQPtrs(IntAnimator)
classQPtrs(ColorAnimator)
classQPtrs(ComplexAnimator)
classQPtrs(BoolPropertyContainer)

classQPtrs(VectorPathAnimator)
classQPtrs(PathAnimator)
classQPtrs(Gradient)
classQPtrs(ColorAnimator)
classQPtrs(PaintSettings)
classQPtrs(StrokeSettings)
classQPtrs(Bone)
classQPtrs(AnimationBox)
classQPtrs(ParticleEmitter)
classQPtrs(InternalLinkBox)
classQPtrs(BoxTargetProperty)

classQPtrs(EffectAnimators)
classQPtrs(PixmapEffect)

classQPtrs(PathEffectAnimators)
classQPtrs(PathEffect)
classQPtrs(DisplacePathEffect)
classQPtrs(DuplicatePathEffect)
classQPtrs(LengthPathEffect)
classQPtrs(SolidifyPathEffect)

classQPtrs(InternalLinkGroupBox)
classQPtrs(SingleSound)
classQPtrs(SoundComposition)
classQPtrs(GradientPoints)

classQPtrs(DurationRectangle)
classQPtrs(VaryingLenAnimationRect)

classQPtrs(BasicTransformAnimator)
classQPtrs(BoxTransformAnimator)
classQPtrs(BoneTransformAnimator)

classPtrs(OutputSettingsProfile)

classPtrs(PaintSetting)
classPtrs(SvgNodePoint)
classPtrs(SvgSeparatePath)

classPtrs(VectorPathEdge)

classPtrs(MinimalExecutor)
classPtrs(_ScheduledExecutor)
classPtrs(FileCacheHandler)
classPtrs(ImageCacheHandler)
classPtrs(AnimationCacheHandler)
classPtrs(ImageSequenceCacheHandler)
classPtrs(VideoCacheHandler)
classPtrs(SoundCacheHandler)

classPtrs(CacheContainer)
structPtrs(FunctionWaitingForBoxLoad)
classPtrs(AnimatorUpdater)
classPtrs(SingleWidgetAbstraction)

classPtrs(TilesData)
classPtrs(SurfaceKey)
classQPtrs(AnimatedSurface)

classPtrs(Key)
classPtrs(PathKey)
classPtrs(QrealKey)
classPtrs(ComplexKey)
classPtrs(QStringKey)

classPtrs(ClipboardContainer)
classPtrs(KeysClipboardContainer)
classPtrs(BoxesClipboardContainer)
classPtrs(PropertyClipboardContainer)

classPtrs(MovablePoint)
classPtrs(PointAnimatorMovablePoint)
classPtrs(CircleCenterPoint)
classPtrs(CircleRadiusPoint)
classPtrs(PathPivot)
classPtrs(QrealPoint)
classPtrs(GradientPoint)
classPtrs(NodePoint)
classPtrs(BonePt)
classPtrs(CtrlPoint)
classPtrs(BoxPathPoint)

structPtrs(NodeSettings)

structPtrs(BoundingBoxRenderData)
structPtrs(ParticleBoxRenderData)
structPtrs(PixmapEffectRenderData)
structPtrs(BlurEffectRenderData)
structPtrs(ShadowEffectRenderData)
structPtrs(LinesEffectRenderData)
structPtrs(CirclesEffectRenderData)
structPtrs(SwirlEffectRenderData)
structPtrs(OilEffectRenderData)
structPtrs(ImplodeEffectRenderData)
structPtrs(DesaturateEffectRenderData)
structPtrs(ColorizeEffectRenderData)
structPtrs(ReplaceColorEffectRenderData)
structPtrs(ContrastEffectRenderData)
structPtrs(BrightnessEffectRenderData)
structPtrs(SampledMotionBlurEffectRenderData)
structPtrs(BrushEffectRenderData)

classPtrs(RenderContainer)

classPtrs(MultiplyTransformCustomizer)
classPtrs(ReplaceTransformDisplacementCustomizer)
classPtrs(RenderDataCustomizerFunctor)

#endif // SHAREDPOINTERDEFS_H
