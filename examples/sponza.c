#include "./common.h"
#include <r3d.h>


/* === Resources === */

static Model		sponza = { 0 };
static R3D_Skybox	skybox = { 0 };
static Camera3D		camera = { 0 };


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight());
    SetTargetFPS(60);

    R3D_SetSSAO(true);
    R3D_SetSSAORadius(4.0f);

    sponza = RES_LoadModel("sponza.glb");

    for (int i = 0; i < sponza.materialCount; i++) {
        sponza.materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        sponza.materials[i].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
        sponza.materials[i].maps[MATERIAL_MAP_ROUGHNESS].value = 1.0f;
        sponza.materials[i].maps[MATERIAL_MAP_METALNESS].value = 1.0f;

        GenTextureMipmaps(&sponza.materials[i].maps[MATERIAL_MAP_ALBEDO].texture);
        SetTextureFilter(sponza.materials[i].maps[MATERIAL_MAP_ALBEDO].texture, TEXTURE_FILTER_TRILINEAR);

        GenTextureMipmaps(&sponza.materials[i].maps[MATERIAL_MAP_NORMAL].texture);
        SetTextureFilter(sponza.materials[i].maps[MATERIAL_MAP_NORMAL].texture, TEXTURE_FILTER_TRILINEAR);
    }

    skybox = R3D_LoadSkybox(RESOURCES_PATH "sky/skybox3.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_EnableSkybox(skybox);

    R3D_Light light = R3D_CreateLight();
    R3D_SetLightDirection(light, (Vector3) { 0, -1, 0 });
    R3D_SetLightActive(light, true);

    camera = (Camera3D){
        .position = (Vector3) { 0, 0, 0 },
        .target = (Vector3) { 0, 0, -1 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    DisableCursor();

    return "[r3d] - sponza example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_FREE);
}

void Draw(void)
{
    R3D_Begin(camera);
        R3D_DrawModel(sponza, (Vector3) { 0 }, 1.0f);
    R3D_End();

    DrawFPS(10, 10);
}

void Close(void)
{
    UnloadModel(sponza);
    R3D_UnloadSkybox(skybox);
    R3D_Close();
}
