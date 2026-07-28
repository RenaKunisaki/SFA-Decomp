#ifndef MAIN_WORLDPLANET_LIGHTING_H_
#define MAIN_WORLDPLANET_LIGHTING_H_

#include "global.h"
#include "game/objects/object.h"

typedef struct WorldPlanetColorRGBA8
{
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} WorldPlanetColorRGBA8;

typedef struct WorldPlanetPaddedColorRGBA8
{
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 padding[4];
} WorldPlanetPaddedColorRGBA8;

STATIC_ASSERT(sizeof(WorldPlanetColorRGBA8) == 4);
STATIC_ASSERT(sizeof(WorldPlanetPaddedColorRGBA8) == 8);

extern WorldPlanetColorRGBA8 gWorldPlanetLightFrom;
extern WorldPlanetColorRGBA8 gWorldPlanetLightTo;
extern WorldPlanetColorRGBA8 gWorldPlanetSkyColorFrom;
extern WorldPlanetColorRGBA8 gWorldPlanetSkyColorTo;
extern WorldPlanetColorRGBA8 gWorldPlanetAmbientFrom;
extern WorldPlanetPaddedColorRGBA8 gWorldPlanetAmbientTo;

extern WorldPlanetColorRGBA8 gWorldPlanetCurAmbient;
extern WorldPlanetColorRGBA8 gWorldPlanetCurLight;
extern WorldPlanetColorRGBA8 gWorldPlanetCurSky;

void worldplanet_updateMapLighting(GameObject* obj);

#endif /* MAIN_WORLDPLANET_LIGHTING_H_ */
