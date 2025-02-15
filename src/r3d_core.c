/*
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "r3d.h"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <glad.h>

#include <float.h>

#include "./r3d_state.h"
#include "./details/r3d_light.h"
#include "./details/r3d_drawcall.h"
#include "./details/r3d_primitives.h"
#include "./details/containers/r3d_array.h"
#include "./details/containers/r3d_registry.h"


/* === Public functions === */

void R3D_Init(int resWidth, int resHeight)
{
    // Load framebuffers
    r3d_framebuffer_load_gbuffer(resWidth, resHeight);
    r3d_framebuffer_load_lit(resWidth, resHeight);
    r3d_framebuffer_load_post(resWidth, resHeight);

    // Load containers
    R3D.container.drawCallArray = r3d_array_create(256, sizeof(r3d_drawcall_t));
    R3D.container.lightRegistry = r3d_registry_create(8, sizeof(r3d_light_t));

    // Load generation shaders
    r3d_shader_load_generate_cubemap_from_equirectangular();
    r3d_shader_load_generate_irradiance_convolution();
    r3d_shader_load_generate_prefilter();

    // Load raster shaders
    r3d_shader_load_raster_geometry();
    r3d_shader_load_raster_skybox();

    // Load screen shaders
    r3d_shader_load_screen_lighting();
    r3d_shader_load_screen_bloom();
    r3d_shader_load_screen_fog();
    r3d_shader_load_screen_tonemap();
    r3d_shader_load_screen_adjustment();

    // Environment data
    R3D.env.backgroundColor = (Vector3) { 0.2f, 0.2f, 0.2f };
    R3D.env.ambientColor = (Vector3) { 0.2f, 0.2f, 0.2f };
    R3D.env.quatSky = QuaternionIdentity();
    R3D.env.useSky = false;
    R3D.env.bloomMode = R3D_BLOOM_DISABLED;
    R3D.env.bloomIntensity = 1.0f;
    R3D.env.bloomHdrThreshold = 1.0f;
    R3D.env.fogMode = R3D_FOG_DISABLED;
    R3D.env.fogColor = (Vector3) { 1.0f, 1.0f, 1.0f };
    R3D.env.fogStart = 5.0f;
    R3D.env.fogEnd = 100.0f;
    R3D.env.fogDensity = 1.0f;
    R3D.env.tonemapMode = R3D_TONEMAP_LINEAR;
    R3D.env.tonemapExposure = 1.0f;
    R3D.env.tonemapWhite = 1.0f;
    R3D.env.brightness = 1.0f;
    R3D.env.contrast = 1.0f;
    R3D.env.saturation = 1.0f;

    // Init state data
    R3D.state.resolutionW = resWidth;
    R3D.state.resolutionH = resHeight;

    // Load default textures
    r3d_texture_load_white();
    r3d_texture_load_black();
    r3d_texture_load_normal();
    r3d_texture_load_ibl_brdf_lut();

    // Load primitive shapes
    R3D.primitive.quad = r3d_primitive_load_quad();
    R3D.primitive.cube = r3d_primitive_load_cube();

    // Init misc data
    R3D.misc.matCubeViews[0] = MatrixLookAt((Vector3) { 0 }, (Vector3) {  1.0f,  0.0f,  0.0f }, (Vector3) { 0.0f, -1.0f,  0.0f });
    R3D.misc.matCubeViews[1] = MatrixLookAt((Vector3) { 0 }, (Vector3) { -1.0f,  0.0f,  0.0f }, (Vector3) { 0.0f, -1.0f,  0.0f });
    R3D.misc.matCubeViews[2] = MatrixLookAt((Vector3) { 0 }, (Vector3) {  0.0f,  1.0f,  0.0f }, (Vector3) { 0.0f,  0.0f,  1.0f });
    R3D.misc.matCubeViews[3] = MatrixLookAt((Vector3) { 0 }, (Vector3) {  0.0f, -1.0f,  0.0f }, (Vector3) { 0.0f,  0.0f, -1.0f });
    R3D.misc.matCubeViews[4] = MatrixLookAt((Vector3) { 0 }, (Vector3) {  0.0f,  0.0f,  1.0f }, (Vector3) { 0.0f, -1.0f,  0.0f });
    R3D.misc.matCubeViews[5] = MatrixLookAt((Vector3) { 0 }, (Vector3) {  0.0f,  0.0f, -1.0f }, (Vector3) { 0.0f, -1.0f,  0.0f });
}

void R3D_Close(void)
{
    r3d_framebuffer_unload_gbuffer();
    r3d_framebuffer_unload_lit();
    r3d_framebuffer_unload_post();

    r3d_array_destroy(&R3D.container.drawCallArray);
    r3d_registry_destroy(&R3D.container.lightRegistry);

    rlUnloadShaderProgram(R3D.shader.generate.cubemapFromEquirectangular.id);
    rlUnloadShaderProgram(R3D.shader.generate.irradianceConvolution.id);
    rlUnloadShaderProgram(R3D.shader.generate.prefilter.id);
    rlUnloadShaderProgram(R3D.shader.raster.geometry.id);
    rlUnloadShaderProgram(R3D.shader.raster.skybox.id);
    rlUnloadShaderProgram(R3D.shader.screen.lighting.id);
    rlUnloadShaderProgram(R3D.shader.screen.bloom.id);
    rlUnloadShaderProgram(R3D.shader.screen.fog.id);
    rlUnloadShaderProgram(R3D.shader.screen.tonemap.id);
    rlUnloadShaderProgram(R3D.shader.screen.adjustment.id);

    rlUnloadTexture(R3D.texture.white);
    rlUnloadTexture(R3D.texture.black);
    rlUnloadTexture(R3D.texture.normal);
    rlUnloadTexture(R3D.texture.iblBrdfLut);

    r3d_primitive_unload(&R3D.primitive.quad);
    r3d_primitive_unload(&R3D.primitive.cube);
}

void R3D_Begin(Camera3D camera)
{
    // Render the batch before proceeding
    rlDrawRenderBatchActive();

    // Clear the previous draw call array state
    r3d_array_clear(&R3D.container.drawCallArray);

    // Store camera position
    R3D.state.posView = camera.position;

    // Compute projection matrix
    float aspect = GetRenderWidth() / (float)GetRenderHeight();
    if (camera.projection == CAMERA_PERSPECTIVE) {
        double top = rlGetCullDistanceNear() * tan(camera.fovy * 0.5 * DEG2RAD);
        double right = top * aspect;

        R3D.state.matProj = MatrixFrustum(
            -right, right, -top, top,
            rlGetCullDistanceNear(),
            rlGetCullDistanceFar()
        );
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
        double top = camera.fovy / 2.0;
        double right = top * aspect;

        R3D.state.matProj = MatrixOrtho(
            -right, right, -top, top,
            rlGetCullDistanceNear(),
            rlGetCullDistanceFar()
        );
    }

    // Compute view matrix
    R3D.state.matView = MatrixLookAt(
        camera.position,
        camera.target,
        camera.up
    );
}

void R3D_End(void)
{
    // [PART 1] - Init global state
    {
        rlDisableColorBlend();
    }

    // [PART 2] - Raster geometries to the geometry buffers
    {
        rlEnableFramebuffer(R3D.framebuffer.gBuffer.id);
        {
            // Clear framebuffer
            {
                glClearBufferfv(GL_DEPTH, 0, (float[4]) {
                    FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX
                });

                glClearBufferfv(GL_COLOR, 0, (float[4]) {
                    R3D.env.backgroundColor.x,
                    R3D.env.backgroundColor.y,
                    R3D.env.backgroundColor.z,
                    1.0f
                });

                glClearBufferfv(GL_COLOR, 1, (float[4]) { 0 });
                glClearBufferfv(GL_COLOR, 2, (float[4]) { 0 });
                glClearBufferfv(GL_COLOR, 3, (float[4]) { 0 });
                glClearBufferfv(GL_COLOR, 4, (float[4]) { 0 });
                glClearBufferfv(GL_COLOR, 5, (float[4]) { 0 });
            }

            // Setup projection matrix
            rlMatrixMode(RL_PROJECTION);
            rlPushMatrix();
            rlSetMatrixProjection(R3D.state.matProj);

            // Setup view matrix
            rlMatrixMode(RL_MODELVIEW);
            rlLoadIdentity();
            rlMultMatrixf(MatrixToFloat(R3D.state.matView));

            // Render skybox - (albedo buffer only)
            if (R3D.env.useSky)
            {
                r3d_shader_enable(raster.skybox);
                rlDisableBackfaceCulling();
                rlDisableDepthMask();

                Matrix matView = rlGetMatrixModelview();
                Matrix matProj = rlGetMatrixProjection();

                // Bind cubemap texture
                r3d_texture_bind_cubemap(0, R3D.env.sky.cubemap.id);

                // Set skybox parameters
                r3d_shader_set_vec4(raster.skybox, uRotation, R3D.env.quatSky);

                // Try binding vertex array objects (VAO) or use VBOs if not possible
                if (!rlEnableVertexArray(R3D.primitive.cube.vao)) {
                    rlEnableVertexBuffer(R3D.primitive.cube.vbo);
                    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
                    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
                    rlEnableVertexBufferElement(R3D.primitive.cube.ebo);
                }

                // Draw skybox (supporting stereo rendering)
                if (rlIsStereoRenderEnabled()) {
                    for (int eye = 0; eye < 2; eye++) {
                        rlViewport(eye * rlGetFramebufferWidth() / 2, 0, rlGetFramebufferWidth() / 2, rlGetFramebufferHeight());
                        r3d_shader_set_mat4(raster.skybox, uMatView, MatrixMultiply(matView, rlGetMatrixViewOffsetStereo(eye)));
                        r3d_shader_set_mat4(raster.skybox, uMatProj, rlGetMatrixProjectionStereo(eye));
                        rlDrawVertexArrayElements(0, 36, 0);
                    }
                }
                else {
                    r3d_shader_set_mat4(raster.skybox, uMatView, matView);
                    r3d_shader_set_mat4(raster.skybox, uMatProj, matProj);
                    rlDrawVertexArrayElements(0, 36, 0);
                }

                // Unbind cubemap texture
                r3d_texture_unbind_cubemap(0);

                // Disable all possible vertex array objects (or VBOs)
                rlDisableVertexArray();
                rlDisableVertexBuffer();
                rlDisableVertexBufferElement();

                // Disable shader program
                rlDisableShader();

                rlEnableBackfaceCulling();
                rlEnableDepthMask();
            }

            // Render meshes
            {
                rlEnableDepthTest();

                r3d_shader_enable(raster.geometry);
                {
                    for (int i = 0; i < R3D.container.drawCallArray.count; i++) {
                        r3d_drawcall_raster_geometry(
                            (r3d_drawcall_t*)R3D.container.drawCallArray.data + i
                        );
                    }
                }
                r3d_shader_disable();

                rlDisableDepthTest();
            }

            // Reset projection matrix
            rlMatrixMode(RL_PROJECTION);
            rlPopMatrix();

            // Reset view matrix
            rlMatrixMode(RL_MODELVIEW);
            rlLoadIdentity();
        }
        rlDisableFramebuffer();
    }

    // [PART 3] - Determine what light should lit the visible scene
    r3d_light_t* lights[8];
    int lightCount = 0;
    {
        for (int id = 1; id <= r3d_registry_get_allocated_count(&R3D.container.lightRegistry); id++) {
            if (!r3d_registry_is_valid(&R3D.container.lightRegistry, id)) continue;
            r3d_light_t* light = r3d_registry_get(&R3D.container.lightRegistry, id);
            if (!light->enabled) continue;
            lights[lightCount++] = light;
        }
    }

    // [PART 4] - Lighting computation from G-buffer data into the final render target
    {
        rlEnableFramebuffer(R3D.framebuffer.lit.id);
        {
            r3d_shader_enable(screen.lighting);
            {
                for (int i = 0; i < lightCount; i++) {
                    r3d_light_t* light = lights[i];

                    // Send common data
                    r3d_shader_set_vec3(screen.lighting, uLights[i].color, light->color);
                    r3d_shader_set_float(screen.lighting, uLights[i].energy, light->energy);
                    r3d_shader_set_int(screen.lighting, uLights[i].type, light->type);
                    r3d_shader_set_int(screen.lighting, uLights[i].enabled, true);

                    // Send specific data
                    if (light->type == R3D_LIGHT_DIR) {
                        r3d_shader_set_vec3(screen.lighting, uLights[i].direction, light->direction);
                    }
                    else if (light->type == R3D_LIGHT_SPOT) {
                        r3d_shader_set_vec3(screen.lighting, uLights[i].position, light->position);
                        r3d_shader_set_vec3(screen.lighting, uLights[i].direction, light->direction);
                        r3d_shader_set_float(screen.lighting, uLights[i].range, light->range);
                        r3d_shader_set_float(screen.lighting, uLights[i].attenuation, light->attenuation);
                        r3d_shader_set_float(screen.lighting, uLights[i].innerCutOff, light->innerCutOff);
                        r3d_shader_set_float(screen.lighting, uLights[i].outerCutOff, light->outerCutOff);
                    }
                    else if (light->type == R3D_LIGHT_OMNI) {
                        r3d_shader_set_vec3(screen.lighting, uLights[i].position, light->position);
                        r3d_shader_set_float(screen.lighting, uLights[i].range, light->range);
                        r3d_shader_set_float(screen.lighting, uLights[i].attenuation, light->attenuation);
                    }
                }

                for (int i = lightCount; i < R3D_SHADER_NUM_LIGHTS; i++) {
                    r3d_shader_set_int(screen.lighting, uLights[i].enabled, false);
                }

                if (R3D.env.useSky) {
                    r3d_texture_bind_cubemap(6, R3D.env.sky.irradiance.id);
                    r3d_texture_bind_cubemap(7, R3D.env.sky.prefilter.id);
                    r3d_texture_bind_2D(8, R3D.texture.iblBrdfLut);

                    r3d_shader_set_vec4(screen.lighting, uQuatSkybox, R3D.env.quatSky);
                    r3d_shader_set_int(screen.lighting, uHasSkybox, true);
                }
                else {
                    r3d_shader_set_vec3(screen.lighting, uColAmbient, R3D.env.ambientColor);
                    r3d_shader_set_int(screen.lighting, uHasSkybox, false);
                }

                r3d_shader_set_mat4(screen.lighting, uMatInvProj, MatrixInvert(R3D.state.matProj));
                r3d_shader_set_mat4(screen.lighting, uMatInvView, MatrixInvert(R3D.state.matView));
                r3d_shader_set_vec3(screen.lighting, uViewPosition, R3D.state.posView);

                r3d_shader_set_float(screen.lighting, uBloomHdrThreshold, R3D.env.bloomHdrThreshold);

                r3d_texture_bind_2D(0, R3D.framebuffer.gBuffer.albedo);
                r3d_texture_bind_2D(1, R3D.framebuffer.gBuffer.emission);
                r3d_texture_bind_2D(2, R3D.framebuffer.gBuffer.normal);
                r3d_texture_bind_2D(3, R3D.framebuffer.gBuffer.depth);
                r3d_texture_bind_2D(4, R3D.framebuffer.gBuffer.orm);
                r3d_texture_bind_2D(5, R3D.framebuffer.gBuffer.matId);

                r3d_primitive_draw_quad();

                r3d_texture_unbind_2D(0);
                r3d_texture_unbind_2D(1);
                r3d_texture_unbind_2D(2);
                r3d_texture_unbind_2D(3);
                r3d_texture_unbind_2D(4);
                r3d_texture_unbind_2D(5);

                if (R3D.env.useSky) {
                    r3d_texture_unbind_cubemap(6);
                    r3d_texture_unbind_cubemap(7);
                    r3d_texture_unbind_2D(8);
                }
            }
            rlDisableShader();
        }
        rlDisableFramebuffer();
    }

    // [PART 5] - Post proccesses using ping-pong buffer
    {
        int texIndex = 2;
        unsigned int textures[3] = {
            R3D.framebuffer.post.textures[0],
            R3D.framebuffer.post.textures[1],
            R3D.framebuffer.lit.color
        };

        rlEnableFramebuffer(R3D.framebuffer.post.id);
        {
            // Post process: Bloom
            if (R3D.env.bloomMode != R3D_BLOOM_DISABLED) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, textures[!texIndex], 0
                );
                r3d_shader_enable(screen.bloom);
                {
                    r3d_texture_bind_2D(0, textures[texIndex]);
                    //r3d_texture_bind_2D(1, BLOOM_BLUR_TEX);
                    texIndex = !texIndex;

                    r3d_shader_set_int(screen.bloom, uBloomMode, R3D.env.bloomMode);
                    r3d_shader_set_float(screen.bloom, uBloomIntensity, R3D.env.bloomIntensity);

                    r3d_primitive_draw_quad();
                }
                r3d_shader_disable();
            }

            // Post process: Fog
            if (R3D.env.fogMode != R3D_FOG_DISABLED) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, textures[!texIndex], 0
                );
                r3d_shader_enable(screen.fog);
                {
                    r3d_texture_bind_2D(0, textures[texIndex]);
                    r3d_texture_bind_2D(1, R3D.framebuffer.gBuffer.depth);
                    texIndex = !texIndex;

                    r3d_shader_set_float(screen.fog, uNear, rlGetCullDistanceNear());
                    r3d_shader_set_float(screen.fog, uFar, rlGetCullDistanceFar());
                    r3d_shader_set_int(screen.fog, uFogMode, R3D.env.fogMode);
                    r3d_shader_set_vec3(screen.fog, uFogColor, R3D.env.fogColor);
                    r3d_shader_set_float(screen.fog, uFogStart, R3D.env.fogStart);
                    r3d_shader_set_float(screen.fog, uFogEnd, R3D.env.fogEnd);
                    r3d_shader_set_float(screen.fog, uFogDensity, R3D.env.fogDensity);

                    r3d_primitive_draw_quad();
                }
                r3d_shader_disable();
            }

            // Post process: Tonemap
            if (R3D.env.tonemapMode != R3D_TONEMAP_LINEAR || R3D.env.tonemapExposure != 1.0f || R3D.env.tonemapWhite != 1.0f) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, textures[!texIndex], 0
                );
                r3d_shader_enable(screen.tonemap);
                {
                    r3d_texture_bind_2D(0, textures[texIndex]);
                    texIndex = !texIndex;

                    r3d_shader_set_int(screen.tonemap, uTonemapMode, R3D.env.tonemapMode);
                    r3d_shader_set_float(screen.tonemap, uTonemapExposure, R3D.env.tonemapExposure);
                    r3d_shader_set_float(screen.tonemap, uTonemapWhite, R3D.env.tonemapWhite);

                    r3d_primitive_draw_quad();
                }
                r3d_shader_disable();
            }

            // Post process: Adjustment
            {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, textures[!texIndex], 0
                );
                r3d_shader_enable(screen.adjustment);
                {
                    r3d_texture_bind_2D(0, textures[texIndex]);
                    texIndex = !texIndex;

                    r3d_shader_set_float(screen.adjustment, uBrightness, R3D.env.brightness);
                    r3d_shader_set_float(screen.adjustment, uContrast, R3D.env.contrast);
                    r3d_shader_set_float(screen.adjustment, uSaturation, R3D.env.saturation);

                    r3d_primitive_draw_quad();
                }
                r3d_shader_disable();
            }
        }
        rlDisableFramebuffer();
    }

    // [PART 6] - Blit the final result to the main framebuffer
    {
        rlBindFramebuffer(RL_READ_FRAMEBUFFER, R3D.framebuffer.post.id);
        rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);

        rlBlitFramebuffer(
            0, 0, R3D.state.resolutionW, R3D.state.resolutionH,
            0, 0, GetScreenWidth(), GetScreenHeight(),
            GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT
        );
    }

    // [PART 7] - Reset global state
    {
        rlEnableColorBlend();
    }
}

void R3D_DrawMesh(Mesh mesh, Material material, Matrix transform)
{
    r3d_drawcall_t drawCall = {
        .transform = transform,
        .material = material,
        .mesh = mesh,
    };

    r3d_array_push_back(
        &R3D.container.drawCallArray,
        &drawCall
    );
}

void R3D_DrawModel(Model model, Vector3 position, float scale)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    R3D_DrawModelEx(model, position, rotationAxis, 0.0f, vScale);
}

void R3D_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale)
{
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);
    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++) {
        R3D_DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
    }
}
