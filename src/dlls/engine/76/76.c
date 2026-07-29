/*
 * DLL 76 / 0x4C.
 */
#include "main/resource.h"
#include "main/object_transform.h"
#include "main/dll/dll_004C_camDebug.h"

void CameraModeFixed_copyToCurrent(void)
{
}

void CameraModeFixed_free(void)
{
}

void CameraModeFixed_update(void)
{
}

void CameraModeFixed_init(CameraObject* camera, int unused, CameraObject* src)
{
    if (src != NULL)
    {
        camera->anim.worldPosX = src->anim.worldPosX;
        camera->anim.worldPosY = src->anim.worldPosY;
        camera->anim.worldPosZ = src->anim.worldPosZ;
        Obj_TransformWorldPointToLocal(src->anim.worldPosX, src->anim.worldPosY, src->anim.worldPosZ,
                                       &camera->anim.localPosX, &camera->anim.localPosY, &camera->anim.localPosZ,
                                       camera->anim.parent);
        camera->anim.rotX = src->anim.rotX;
        camera->anim.rotY = src->anim.rotY;
        camera->anim.rotZ = src->anim.rotZ;
        camera->fov = src->fov;
    }
}

void CameraModeFixed_release(void)
{
}

void CameraModeFixed_initialise(void)
{
}

ResourceDescriptorCallbacks8 lbl_80319D78 = {{0x00000000,
                       0x00000000,
                       0x00000000,
                       0x00060000},
                      {(ResourceDescriptorCallback)CameraModeFixed_initialise,
                       (ResourceDescriptorCallback)CameraModeFixed_release,
                       0x00000000,
                       (ResourceDescriptorCallback)CameraModeFixed_init,
                       (ResourceDescriptorCallback)CameraModeFixed_update,
                       (ResourceDescriptorCallback)CameraModeFixed_free,
                       (ResourceDescriptorCallback)CameraModeFixed_copyToCurrent,
                       0x00000000}};
