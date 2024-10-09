#ifndef SHIP_UTILS_H
#define SHIP_UTILS_H

#include <libultraship/libultraship.h>
#include "PR/ultratypes.h"

#ifdef __cplusplus

#include <string>

void LoadGuiTextures();

extern "C" {
#endif

bool Ship_IsCStringEmpty(const char* str);
void Ship_CreateQuadVertexGroup(Vtx* vtxList, s32 xStart, s32 yStart, s32 width, s32 height, u8 flippedH);
f32 Ship_GetCharFontWidthNES(u8 character);
TexturePtr Ship_GetCharFontTextureNES(u8 character);

#ifdef __cplusplus
}
bool isStringEmpty(std::string str);
const char* getSceneName(s16 sceneId);
#endif

#endif // SHIP_UTILS_H
