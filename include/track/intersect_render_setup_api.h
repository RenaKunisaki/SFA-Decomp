#ifndef TRACK_INTERSECT_RENDER_SETUP_API_H_
#define TRACK_INTERSECT_RENDER_SETUP_API_H_

#include "types.h"
#include "main/projected_shadow.h"

void setupAdditiveTintedTexture(void* texture, u32* colorA, u32* colorB);
void gxDebugTextureFn_80078c1c(void);
void gxTevModulateRasStage(void);
void gxTevRasTimesColor1Stage(void);
void gxTevPassRasStage(void);
void _gxSetTevColor1(int r, int g, int b, int a);
void _gxSetTevColor2(int r, int g, int b, int a);
void gxTevAddTextureFrameBlendStages(void);
void setupReflectionIndirectTev(u8 flag);
void objectShadow_setupSwappedProjectedTexture(ProjectedShadowTexture* shadow, u32* color, f32 mtx[3][4]);
void objectShadow_setupProjectedTexture(ProjectedShadowTexture* shadow, u32* color, f32 mtx[3][4]);
void objectShadow_setupProjectedTextureDepthFade(ProjectedShadowTexture* shadow, u32* color, f32 mtx[3][4], f32 depth);
void objectShadow_setupProjectedTextureChannel(ProjectedShadowTexture* shadow, u32* color, f32 mtx[3][4], f32 scale);
void textRenderSetupFn_80079804(void);
void textureSetupFn_800799c0(void);

#endif /* TRACK_INTERSECT_RENDER_SETUP_API_H_ */
