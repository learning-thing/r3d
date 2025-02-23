#include "./common.h"

/* === Resources === */

static Mesh         sphere = { 0 };
static R3D_Skybox   skybox = { 0 };
static Camera3D     camera = { 0 };

static Material materials[7 * 7] = { 0 };


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    sphere = GenMeshSphere(0.5f, 64, 64);

    for (int x = 0; x < 7; x++) {
        for (int y = 0; y < 7; y++) {
            int i = y * 7 + x;
            materials[i] = LoadMaterialDefault();
            R3D_SetMaterialOcclusion(&materials[i], NULL, 1.0f);
            R3D_SetMaterialMetalness(&materials[i], NULL, (float)x / 7);
            R3D_SetMaterialRoughness(&materials[i], NULL, (float)y / 7);
            R3D_SetMaterialAlbedo(&materials[i], NULL, ColorFromHSV(((float)x/7) * 360, 1, 1));
        }
    }

    skybox = R3D_LoadSkybox(RESOURCES_PATH "sky/skybox1.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_EnableSkybox(skybox);

    camera = (Camera3D){
        .position = (Vector3) { 0, 0, 5 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    DisableCursor();

    return "[r3d] - skybox example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_FREE);
}

void Draw(void)
{
    R3D_Begin(camera);
        for (int x = 0; x < 7; x++) {
            for (int y = 0; y < 7; y++) {
                R3D_DrawMesh(sphere, materials[y * 7 + x], MatrixTranslate(x - 3, y - 3, 0.0f));
            }
        }
    R3D_End();
}

void Close(void)
{
    for (int i = 0; i < 7 * 7; i++) {
        RL_FREE(materials[i].maps);
    }
    UnloadMesh(sphere);
    R3D_UnloadSkybox(skybox);
    R3D_Close();
}
