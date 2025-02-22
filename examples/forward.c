#include "./common.h"
#include <r3d.h>


/* === Resources === */

static Model		cube = { 0 };
static Model		plane = { 0 };
static Model		sphere = { 0 };
static Camera3D		camera = { 0 };


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    cube = LoadModelFromMesh(GenMeshCube(1, 1, 1));
    cube.materials[0].maps[MATERIAL_MAP_ALBEDO].color = (Color){ 100, 100, 255, 100 };
    cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.2;
    cube.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.2;

    plane = LoadModelFromMesh(GenMeshPlane(1000, 1000, 1, 1));
    plane.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0;

    sphere = LoadModelFromMesh(GenMeshSphere(0.5f, 64, 64));
    sphere.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    sphere.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.25;
    sphere.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.75;

    camera = (Camera3D){
        .position = (Vector3) { 0, 2, 2 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    R3D_Light light = R3D_CreateLight(R3D_LIGHT_SPOT);
    {
        R3D_SetLightPosition(light, (Vector3) { 0, 10, 5 });
        R3D_SetLightTarget(light, (Vector3) { 0 });
        R3D_SetLightActive(light, true);
        R3D_EnableShadow(light, 4096);
    }

    return "[r3d] - forward example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_ORBITAL);
}

void Draw(void)
{
    R3D_Begin(camera);
    {
        R3D_ApplyRenderMode(R3D_RENDER_DEFERRED);
        R3D_ApplyShadowCasting(true);

        R3D_DrawModel(plane, (Vector3) { 0, -0.5f, 0 }, 1.0f);
        R3D_DrawModel(sphere, (Vector3) { 0 }, 1.0f);

        R3D_ApplyRenderMode(R3D_RENDER_FORWARD);
        R3D_ApplyShadowCasting(false);

        R3D_DrawModel(cube, (Vector3) { 0 }, 1.0f);
    }

    R3D_End();
}

void Close(void)
{
    UnloadModel(plane);
    UnloadModel(sphere);
    R3D_Close();
}
