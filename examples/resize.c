#include "./common.h"
#include <rlgl.h>

/* === Resources === */

static Model		sphere = { 0 };
static Camera3D		camera = { 0 };
static Material     materials[5] = { 0 };


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    sphere = LoadModelFromMesh(GenMeshSphere(0.5f, 64, 64));
    UnloadMaterial(sphere.materials[0]);

    for (int i = 0; i < 5; i++) {
        materials[i] = LoadMaterialDefault();
        materials[i].maps[MATERIAL_MAP_ALBEDO].color = ColorFromHSV((float)i / 5 * 330, 1.0f, 1.0f);
        materials[i].maps[MATERIAL_MAP_OCCLUSION].value = 1;
        materials[i].maps[MATERIAL_MAP_ROUGHNESS].value = 1;
        materials[i].maps[MATERIAL_MAP_METALNESS].value = 0;
    }

    camera = (Camera3D){
        .position = (Vector3) { 0, 2, 2 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
    {
        R3D_SetLightDirection(light, (Vector3) { 0, 0, -1 });
        R3D_SetLightActive(light, true);
    }

    return "[r3d] - resize example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_ORBITAL);

    if (IsKeyPressed(KEY_R)) {
        bool keep = R3D_HasState(R3D_FLAG_ASPECT_KEEP);
        if (keep) R3D_ClearState(R3D_FLAG_ASPECT_KEEP);
        else R3D_SetState(R3D_FLAG_ASPECT_KEEP);
    }

    if (IsKeyPressed(KEY_F)) {
        bool linear = R3D_HasState(R3D_FLAG_BLIT_LINEAR);
        if (linear) R3D_ClearState(R3D_FLAG_BLIT_LINEAR);
        else R3D_SetState(R3D_FLAG_BLIT_LINEAR);
    }
}

void Draw(void)
{
    bool keep = R3D_HasState(R3D_FLAG_ASPECT_KEEP);
    bool linear = R3D_HasState(R3D_FLAG_BLIT_LINEAR);

    if (keep) {
        ClearBackground(BLACK);
    }

    R3D_Begin(camera);
        rlPushMatrix();
        for (int i = 0; i < 5; i++) {
            sphere.materials[0] = materials[i];
            R3D_DrawModel(sphere, (Vector3) { i - 2, 0, 0 }, 1.0f);
        }
        rlPopMatrix();
    R3D_End();

    DrawText(TextFormat("Resize mode: %s", keep ? "KEEP" : "EXPAND"), 10, 10, 20, BLACK);
    DrawText(TextFormat("Filter mode: %s", linear ? "LINEAR" : "NEAREST"), 10, 40, 20, BLACK);
}

void Close(void)
{
    UnloadModel(sphere);
    R3D_Close();
}
