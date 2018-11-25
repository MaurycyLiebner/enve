#ifndef SHAREDPOINTERDEFS_H
#define SHAREDPOINTERDEFS_H

#include <QSharedPointer>
#include <QPointer>
#include <memory>
#include "stdpointer.h"

#define SPtrGetAs(baseObjT, derivedClassT) derivedClassT::getAs<derivedClassT>(baseObjT)
#define SPtrCreate(classT) classT::create<classT>
#define classQPtrs(classT) class classT; \
    typedef QSharedPointer<classT> classT##QSPtr; \
    typedef QWeakPointer<classT> classT##QWPtr; \
    typedef QPointer<classT> classT##QPtr;
#define classPtrs(classT) class classT; \
    typedef std::shared_ptr<classT> classT##SPtr; \
    typedef std::weak_ptr<classT> classT##WPtr; \
    typedef StdPointer<classT> classT##Ptr;
#define structPtrs(structT) struct structT; \
    typedef std::shared_ptr<structT> structT##SPtr; \
    typedef std::weak_ptr<structT> structT##WPtr; \
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

classQPtrs(QPointFAnimator)
classQPtrs(QrealAnimator)
classQPtrs(IntPropertyQSPtr)
classQPtrs(ColorAnimator)
classQPtrs(BoolProperty)
classQPtrs(SingleWidgetTarget)
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
classQPtrs(ComplexAnimator)
classQPtrs(PixmapEffect)
classQPtrs(PathEffect)
classQPtrs(Property)
classQPtrs(BoxTargetProperty)
classQPtrs(QrealAnimator)
classQPtrs(Animator)
classQPtrs(QStringAnimator)
classQPtrs(InternalLinkGroupBox)
classQPtrs(SingleSound)
classQPtrs(SoundComposition)
classQPtrs(GradientPoints)

classQPtrs(DurationRectangle)
classQPtrs(VaryingLenAnimationRect)

classQPtrs(BasicTransformAnimator)
classQPtrs(BoxTransformAnimator)
classQPtrs(BoneTransformAnimator)

classQPtrs(OutputSettingsProfile)

classPtrs(VectorPathEdge)
classPtrs(MinimalExecutor)
classPtrs(_ScheduledExecutor)
classPtrs(CacheContainer)
classPtrs(FunctionWaitingForBoxLoad)
classPtrs(Key)
classPtrs(PathKey)
classPtrs(QrealKey)
classPtrs(ComplexKey)
classPtrs(AnimatorUpdater)
classPtrs(SingleWidgetAbstraction)

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

#endif // SHAREDPOINTERDEFS_H
