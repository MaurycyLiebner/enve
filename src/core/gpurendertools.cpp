// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "gpurendertools.h"

GpuRenderTools::GpuRenderTools(QGL33 * const gl,
                               SwitchableContext &context,
                               sk_sp<SkImage> img) :
    mGL(gl), mContext(context) {
    mContext.switchToSkia();
    if(!img->isTextureBacked())
        img = img->makeTextureImage(context.grContext(), nullptr, GrMipMapped::kNo);
    const auto grTex = img->getBackendTexture(true);
    GrGLTextureInfo texInfo;
    grTex.getGLTextureInfo(&texInfo);
    mSrcTexture.fId = texInfo.fID;
    mSrcTexture.fWidth = grTex.width();
    mSrcTexture.fHeight = grTex.height();
}

GpuRenderTools::~GpuRenderTools() {
    mTargetTextureFbo.clear(mGL, mContext.grContext());
    mSrcTexture.clear(mContext.grContext());
}

GLuint GpuRenderTools::getSquareVAO() const {
    return mContext.textureSquareVAO();
}

void GpuRenderTools::swapTextures() {
    if(!validTargetFbo()) return;
    mTargetTextureFbo.swapTexture(mGL, mSrcTexture);
}

void GpuRenderTools::switchToOpenGL(QGL33 * const gl) {
    mContext.switchToOpenGL(gl);
}

void GpuRenderTools::switchToSkia() {
    mContext.switchToSkia();
}

SkCanvas *GpuRenderTools::requestTargetCanvas() {
    if(!mCanvas) {
        mContext.switchToSkia();
        requestTargetFbo();
        GrGLFramebufferInfo fboInfo;
        fboInfo.fFBOID = mTargetTextureFbo.fFBOId;
        fboInfo.fFormat = GR_GL_RGBA8;
        const auto grFbo = GrBackendRenderTarget(mTargetTextureFbo.fWidth,
                                                 mTargetTextureFbo.fHeight,
                                                 0, 8, fboInfo);
        const auto grContext = mContext.grContext();
        mSurface = SkSurface::MakeFromBackendRenderTarget(
                    grContext, grFbo, kBottomLeft_GrSurfaceOrigin,
                    kRGBA_8888_SkColorType, nullptr, nullptr);
        if(!mSurface) RuntimeThrow("Failed to make SkSurface.");

        mCanvas = mSurface->getCanvas();
    }
    return mCanvas;
}

eTexture &GpuRenderTools::getSrcTexture() {
    return mSrcTexture;
}

eTextureFrameBuffer &GpuRenderTools::requestTargetFbo() {
    if(!validTargetFbo())
        mTargetTextureFbo.gen(mGL, mContext.grContext(),
                              mSrcTexture.fWidth, mSrcTexture.fHeight);
    return mTargetTextureFbo;
}

sk_sp<SkImage> GpuRenderTools::requestSrcTextureImageWrapper() {
    const auto grContext = mContext.grContext();
    if(!grContext) RuntimeThrow("Wrapping texture image with SkImage "
                                "only available in Skia mode.\n"
                                "Use SwitchableContext::switchToSkia "
                                "prior to this call.");
    return SkImage::MakeFromTexture(grContext,
                                    sourceBackedTexture(),
                                    kBottomLeft_GrSurfaceOrigin,
                                    kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType,
                                    nullptr);
}

bool GpuRenderTools::validTargetCanvas() const {
    return mCanvas;
}

bool GpuRenderTools::validTargetFbo() const {
    return mTargetTextureFbo.fFBOId != 0;
}

GrBackendTexture GpuRenderTools::sourceBackedTexture() {
    GrGLTextureInfo texInfo;
    texInfo.fID = mSrcTexture.fId;
    texInfo.fFormat = GR_GL_RGBA8;
    texInfo.fTarget = GR_GL_TEXTURE_2D;
    return GrBackendTexture(mSrcTexture.fWidth, mSrcTexture.fHeight,
                            GrMipMapped::kNo, texInfo);
}
