#ifndef SKIAINCLUDES_H
#define SKIAINCLUDES_H

#include "skiadefines.h"

#undef foreach

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SDL.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkGradientShader.h"
#include "SkPixelRef.h"
#include "SkDashPathEffect.h"
#include "SkTypeface.h"
#include "SkImageFilter.h"
#include "SkDiscretePathEffect.h"
#include "SkPathMeasure.h"

#include "gl/GrGLUtil.h"

#include "gl/GrGLTypes.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLInterface.h"

#include <GL/gl.h>

#include "AddInclude/SkStroke.h"
#include "AddInclude/SkGeometry.h"
#include "AddInclude/SkPointPriv.h"

#define foreach Q_FOREACH

#include "skiahelpers.h"
#include "skqtconversions.h"

#endif // SKIAINCLUDES_H
