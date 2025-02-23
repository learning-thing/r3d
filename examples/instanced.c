#include "./common.h"

#define INSTANCE_COUNT 1000


/* === Resources === */

static Camera3D		camera = { 0 };

static Mesh		    mesh = { 0 };
static Material     material = { 0 };
static Matrix       transforms[INSTANCE_COUNT] = {0};
static Color        colors[INSTANCE_COUNT] = {0};


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    mesh = GenMeshCube(1, 1, 1);

    material = LoadMaterialDefault();
    R3D_SetMaterialOcclusion(&material, NULL, 1.0f);
    R3D_SetMaterialRoughness(&material, NULL, 0.5f);
    R3D_SetMaterialMetalness(&material, NULL, 0.5f);

    //GenMeshTangents(&mesh);

    for (int i = 0; i < INSTANCE_COUNT; i++) {
        Matrix translate = MatrixTranslate(
            (float)GetRandomValue(-50000, 50000) / 1000,
            (float)GetRandomValue(-50000, 50000) / 1000,
            (float)GetRandomValue(-50000, 50000) / 1000
        );
        Matrix rotate = MatrixRotateXYZ((Vector3) {
            (float)GetRandomValue(-314000, 314000) / 100000,
            (float)GetRandomValue(-314000, 314000) / 100000,
            (float)GetRandomValue(-314000, 314000) / 100000
        });
        Matrix scale = MatrixScale(
            (float)GetRandomValue(100, 2000) / 1000,
            (float)GetRandomValue(100, 2000) / 1000,
            (float)GetRandomValue(100, 2000) / 1000
        );
        transforms[i] = MatrixMultiply(MatrixMultiply(scale, rotate), translate);
        colors[i] = ColorFromHSV((float)GetRandomValue(0, 360000) / 1000, 1.0f, 1.0f);
    }

    camera = (Camera3D){
        .position = (Vector3) { 0, 2, 2 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
    {
        R3D_SetLightDirection(light, (Vector3) { 0, -1, 0 });
        R3D_SetLightActive(light, true);
    }

    DisableCursor();

    return "[r3d] - instanced example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_FREE);
}

void Draw(void)
{
    R3D_Begin(camera);
        R3D_DrawMeshInstancedEx(mesh, material, transforms, colors, INSTANCE_COUNT);
    R3D_End();

    DrawFPS(10, 10);
}

void Close(void)
{
    UnloadMaterial(material);
    UnloadMesh(mesh);
    R3D_Close();
}
