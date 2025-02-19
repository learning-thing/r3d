#include "./r3d_light.h"

#include <raymath.h>
#include <stddef.h>
#include <glad.h>

/* === Internal functions === */

static r3d_shadow_map_t r3d_light_create_shadow_map_dir(int resolution)
{
    r3d_shadow_map_t shadowMap = { 0 };

    glGenFramebuffers(1, &shadowMap.id);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.id);

    glGenTextures(1, &shadowMap.depth);
    glBindTexture(GL_TEXTURE_2D, shadowMap.depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.depth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_ERROR, "Framebuffer creation error for the directional shadow map");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowMap.texelSize = 1.0f / resolution;
    shadowMap.resolution = resolution;

    return shadowMap;
}

static r3d_shadow_map_t r3d_light_create_shadow_map_spot(int resolution)
{
    r3d_shadow_map_t shadowMap = { 0 };

    shadowMap.resolution = resolution;

    glGenFramebuffers(1, &shadowMap.id);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.id);

    glGenTextures(1, &shadowMap.depth);
    glBindTexture(GL_TEXTURE_2D, shadowMap.depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap.depth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_ERROR, "Framebuffer creation error for the Shadow Map Spot");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowMap.texelSize = 1.0f / resolution;
    shadowMap.resolution = resolution;

    return shadowMap;
}

static r3d_shadow_map_t r3d_light_create_shadow_map_omni(int resolution)
{
    r3d_shadow_map_t shadowMap = { 0 };

    glGenFramebuffers(1, &shadowMap.id);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.id);

    // Cr�ation du Renderbuffer 2D pour la profondeur
    glGenRenderbuffers(1, &shadowMap.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, shadowMap.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, resolution, resolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, shadowMap.depth);

    // Cr�ation de la Cubemap pour la couleur (1 canal 16-bit)
    glGenTextures(1, &shadowMap.color);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap.color);

    for (int i = 0; i < 6; ++i) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R16F,
            resolution, resolution, 0, GL_RED, GL_FLOAT, NULL
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadowMap.color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, shadowMap.color, 0);

    // D�finition des buffers actifs
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_ERROR, "Framebuffer creation error for the omni shadow map");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    shadowMap.texelSize = 1.0f / resolution;
    shadowMap.resolution = resolution;

    return shadowMap;
}

/* === Public functions === */

void r3d_light_init(r3d_light_t* light)
{
    light->shadow = (r3d_shadow_t){ 0 };
    light->color = (Vector3){ 1, 1, 1 };
    light->position = (Vector3){ 0 };
    light->direction = (Vector3){ 0, 0, -1 };
    light->energy = 1.0f;
    light->range = 100.0f;
    light->attenuation = 1.0f;
    light->innerCutOff = -1.0f;
    light->outerCutOff = -1.0f;
    light->type = R3D_LIGHT_DIR;
    light->enabled = false;
}

void r3d_light_create_shadow_map(r3d_light_t* light, int resolution)
{
    switch (light->type) {
    case R3D_LIGHT_DIR:
        light->shadow.map = r3d_light_create_shadow_map_dir(resolution);
        break;
    case R3D_LIGHT_SPOT:
        light->shadow.map = r3d_light_create_shadow_map_spot(resolution);
        break;
    case R3D_LIGHT_OMNI:
        light->shadow.map = r3d_light_create_shadow_map_omni(resolution);
        break;
    }
}

void r3d_light_destroy_shadow_map(r3d_light_t* light)
{
    if (light->shadow.map.id != 0) {
        rlUnloadTexture(light->shadow.map.depth);
        rlUnloadFramebuffer(light->shadow.map.id);
        if (light->shadow.map.color != 0) {
            rlUnloadTexture(light->shadow.map.color);
        }
    }
}

Matrix r3d_light_get_matrix_view_omni(r3d_light_t* light, int face)
{
    static const Vector3 dirs[6] = {
        {  1.0,  0.0,  0.0 }, // +X
        { -1.0,  0.0,  0.0 }, // -X
        {  0.0,  1.0,  0.0 }, // +Y
        {  0.0, -1.0,  0.0 }, // -Y
        {  0.0,  0.0,  1.0 }, // +Z
        {  0.0,  0.0, -1.0 }  // -Z
    };

    static const Vector3 ups[6] = {
        {  0.0, -1.0,  0.0 }, // +X
        {  0.0, -1.0,  0.0 }, // -X
        {  0.0,  0.0,  1.0 }, // +Y
        {  0.0,  0.0, -1.0 }, // -Y
        {  0.0, -1.0,  0.0 }, // +Z
        {  0.0, -1.0,  0.0 }  // -Z
    };

    return MatrixLookAt(
        light->position, Vector3Add(light->position, dirs[face]), ups[face]
    );
}
