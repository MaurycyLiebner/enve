#ifndef SHAREDPOINTERDEFS_H
#define SHAREDPOINTERDEFS_H

#include <QSharedPointer>
#include <memory>

class QPointFAnimator;
class QrealAnimator;
class ColorAnimator;
class BoolProperty;
typedef QSharedPointer<QPointFAnimator> QPointFAnimatorQSPtr;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;
typedef QSharedPointer<ColorAnimator> ColorAnimatorQSPtr;
typedef QSharedPointer<BoolProperty> BoolPropertyQSPtr;

struct BoundingBoxRenderData;
typedef std::shared_ptr<BoundingBoxRenderData> BoundingBoxRenderDataSPtr;

struct PixmapEffectRenderData;
typedef std::shared_ptr<PixmapEffectRenderData> PixmapEffectRenderDataSPtr;
struct BlurEffectRenderData;
typedef std::shared_ptr<BlurEffectRenderData> BlurEffectRenderDataSPtr;
struct ShadowEffectRenderData;
typedef std::shared_ptr<ShadowEffectRenderData> ShadowEffectRenderDataSPtr;
struct LinesEffectRenderData;
typedef std::shared_ptr<LinesEffectRenderData> LinesEffectRenderDataSPtr;
struct CirclesEffectRenderData;
typedef std::shared_ptr<CirclesEffectRenderData> CirclesEffectRenderDataSPtr;
struct SwirlEffectRenderData;
typedef std::shared_ptr<SwirlEffectRenderData> SwirlEffectRenderDataSPtr;
struct OilEffectRenderData;
typedef std::shared_ptr<OilEffectRenderData> OilEffectRenderDataSPtr;
struct ImplodeEffectRenderData;
typedef std::shared_ptr<ImplodeEffectRenderData> ImplodeEffectRenderDataSPtr;
struct DesaturateEffectRenderData;
typedef std::shared_ptr<DesaturateEffectRenderData> DesaturateEffectRenderDataSPtr;
struct ColorizeEffectRenderData;
typedef std::shared_ptr<ColorizeEffectRenderData> ColorizeEffectRenderDataSPtr;
struct ReplaceColorEffectRenderData;
typedef std::shared_ptr<ReplaceColorEffectRenderData> ReplaceColorEffectRenderDataSPtr;
struct ContrastEffectRenderData;
typedef std::shared_ptr<ContrastEffectRenderData> ContrastEffectRenderDataSPtr;
struct BrightnessEffectRenderData;
typedef std::shared_ptr<BrightnessEffectRenderData> BrightnessEffectRenderDataSPtr;
struct SampledMotionBlurEffectRenderData;
typedef std::shared_ptr<SampledMotionBlurEffectRenderData> SampledMotionBlurEffectRenderDataSPtr;
struct BrushEffectRenderData;
typedef std::shared_ptr<BrushEffectRenderData> BrushEffectRenderDataSPtr;
#endif // SHAREDPOINTERDEFS_H
