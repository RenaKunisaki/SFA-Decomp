#ifndef MAIN_PI_DOLPHIN_API_H_
#define MAIN_PI_DOLPHIN_API_H_

#include "types.h"
#include "main/pi_frame_api.h"
#include "main/pi_dolphin_path_api.h"

double SeekTwiceBeforeRead(void);
void* loadAndDecompressDataFile(int fileId, void* destBuf, int offsetFlags, u32 length, int* sizeOut,
                                int entryIndex, u32 flagBits);
int mapGetDirIdx(int idx);
u8 isHeavyFogEnabled(void);
void disableHeavyFog(void);
void enableHeavyFog(f32 top, f32 bottom, f32 depthScale, f32 depthOffset, f32 worldScale, u8 mode);
void addWavyCausticTevStage(void);
void addTexModulateReg2Stage(void);
void addShadowFalloffTevStages(void);
void addSmallReflectionTevStage(void);
void setColor_803db5d0(u8 r, u8 g, u8 b);
void chooseTevKonstSelectors(void* params, u8 colorEnabled, u8 alphaEnabled, int* colorSelection,
                             int* alphaSelection);
void addYUVVideoTevStages(void* yTexture, void* uTexture, void* vTexture, s16 width, s16 height);
void setHeatEffectParams(u8 alpha, f32 scale);
void* Shader_getLayer(void* shader, int layerIdx);
void setupCausticBaseTevStages(void* viewMtx);
void setupHeatShimmerTevStages(char* shader);
void addWarpedRingTevStages(void);
void addVertexAlphaDimStage(u8* color);
void renderHeavyFog(void* fogColor);
void addLightColorModulateStage(int* color);
void addAccumulatedLightBlendStages(void);
void addAccumulatedLightModulateStage(void);
void addPointLightAccumStages(f32 scale, int* color, f32* position);
void addFirstPointLightStages(f32 scale, int* color, f32* position, u8* chanColor);
void addPointLightDirectStages(f32 scale, int* color, f32* position);
void addSignedOverlayTexStage(u8* texture, void* texMtx, u8* color);
void textureFn_8004ff20(void* texture, f32* texMtx, void* color, int unused);
void addTexLayerStagesLit(void* texture, void* texMtx);
void gxTextureFn_80050e28(u8 mode);
int textureFn_80050ad8(void* texture, int stageCount, u8 mode, u32 indirectTextureId);
void textureFn_80051348(void* textureRef, u8 objectFlags);
void addLightTexReg2Stage(void* textureRef, u8 hasBaseTexture, u8 mode);
void addAlphaLitColorReg2Stage(u8 mode);
void addCastShadowTevStages(u8* objectInstance);
void addProjectedLightTevStage(u8* texture, void* texMtx, int stageMode, int componentMode, int variant);
void addEnvMapTexCoord(int scale);
void textureFn_8004c330(void* texture, void* texMtx);
void gxTextureFn_8004d5b4(void* renderOp);

void mapsBinGetRomlistSize(int idx, int* out1, int* out2, int* out3, int p5);

extern s16 gObjMapBlockInfo[];
extern s16 sMapFileNameAdjacencyTable[];
extern char sAssetIndexOverflowError[];

#endif /* MAIN_PI_DOLPHIN_API_H_ */
