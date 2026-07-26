#include "main/dll/SH/dll_01B0_shswapston.h"

ObjectDescriptor gWarpStoneObjDescriptor = {
    0,
    0,
    0,
    OBJECT_DESCRIPTOR_FLAGS_10_SLOTS,
    (ObjectDescriptorCallback)warpstone_initialise,
    (ObjectDescriptorCallback)warpstone_release,
    0,
    (ObjectDescriptorCallback)warpstone_init,
    (ObjectDescriptorCallback)warpstone_update,
    (ObjectDescriptorCallback)warpstone_hitDetect,
    (ObjectDescriptorCallback)warpstone_render,
    (ObjectDescriptorCallback)warpstone_free,
    (ObjectDescriptorCallback)warpstone_getObjectTypeId,
    warpstone_getExtraSize,
};

#include "../src/main/dll/scchieflightfoot.c"
#include "../src/main/dll/sclantern.c"
#include "../src/main/dll/SH/dll_01B0_shswapston.c"
