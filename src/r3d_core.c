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

// TODO: Load some shaders/textures when needed (like ssao/lut/etc)

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

/* === Internal declarations === */

static void r3d_framebuffers_load(int width, int height);
static void r3d_framebuffers_unload(void);


/* === Public functions === */

void R3D_Init(int resWidth, int resHeight, unsigned int flags)
{
    // Load framebuffers
    r3d_framebuffers_load(resWidth, resHeight);

    // Load containers
    R3D.container.drawCallArray = r3d_array_create(256, sizeof(r3d_drawcall_t));
    R3D.container.lightRegistry = r3d_registry_create(8, sizeof(r3d_light_t));

    // Load generation shaders
    r3d_shader_load_generate_gaussian_blur_dual_pass();
    r3d_shader_load_generate_cubemap_from_equirectangular();
    r3d_shader_load_generate_irradiance_convolution();
    r3d_shader_load_generate_prefilter();

    // Load raster shaders
    r3d_shader_load_raster_geometry();
    r3d_shader_load_raster_skybox();
    r3d_shader_load_raster_depth();
    r3d_shader_load_raster_depth_cube();

    // Load screen shaders
    r3d_shader_load_screen_ssao();
    r3d_shader_load_screen_lighting();
    r3d_shader_load_screen_bloom();
    r3d_shader_load_screen_fog();
    r3d_shader_load_screen_tonemap();
    r3d_shader_load_screen_adjustment();
    r3d_shader_load_screen_fxaa();

    // Environment data
    R3D.env.backgroundColor = (Vector3) { 0.2f, 0.2f, 0.2f };
    R3D.env.ambientColor = (Vector3) { 0.2f, 0.2f, 0.2f };
    R3D.env.quatSky = QuaternionIdentity();
    R3D.env.useSky = false;
    R3D.env.ssaoEnabled = false;
    R3D.env.ssaoRadius = 0.5f;
    R3D.env.ssaoBias = 0.025f;
    R3D.env.ssaoIterations = 10;
    R3D.env.bloomMode = R3D_BLOOM_DISABLED;
    R3D.env.bloomIntensity = 1.0f;
    R3D.env.bloomHdrThreshold = 1.0f;
    R3D.env.bloomIterations = 10;
    R3D.env.fogMode = R3D_FOG_DISABLED;
    R3D.env.fogColor = (Vector3) { 1.0f, 1.0f, 1.0f };
    R3D.env.fogStart = 1.0f;
    R3D.env.fogEnd = 50.0f;
    R3D.env.fogDensity = 0.05f;
    R3D.env.tonemapMode = R3D_TONEMAP_LINEAR;
    R3D.env.tonemapExposure = 1.0f;
    R3D.env.tonemapWhite = 1.0f;
    R3D.env.brightness = 1.0f;
    R3D.env.contrast = 1.0f;
    R3D.env.saturation = 1.0f;

    // Init resolution state
    R3D.state.resolution.width = resWidth;
    R3D.state.resolution.height = resHeight;
    R3D.state.resolution.texelX = 1.0f / resWidth;
    R3D.state.resolution.texelY = 1.0f / resHeight;

    // Init FXAA default state
    R3D.state.fxaa.qualityLevel = 0.5f;
    R3D.state.fxaa.edgeSensitivity = 0.75f;
    R3D.state.fxaa.subpixelQuality = 0.5f;

    // Set parameter flags
    R3D.state.flags = flags;

    // Load default textures
    r3d_texture_load_white();
    r3d_texture_load_black();
    r3d_texture_load_normal();
    r3d_texture_load_ssao_noise();
    r3d_texture_load_ssao_kernel();
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
    r3d_framebuffers_unload();

    r3d_array_destroy(&R3D.container.drawCallArray);
    r3d_registry_destroy(&R3D.container.lightRegistry);

    rlUnloadShaderProgram(R3D.shader.generate.gaussianBlurDualPass.id);
    rlUnloadShaderProgram(R3D.shader.generate.cubemapFromEquirectangular.id);
    rlUnloadShaderProgram(R3D.shader.generate.irradianceConvolution.id);
    rlUnloadShaderProgram(R3D.shader.generate.prefilter.id);
    rlUnloadShaderProgram(R3D.shader.raster.geometry.id);
    rlUnloadShaderProgram(R3D.shader.raster.skybox.id);
    rlUnloadShaderProgram(R3D.shader.raster.depth.id);
    rlUnloadShaderProgram(R3D.shader.raster.depthCube.id);
    rlUnloadShaderProgram(R3D.shader.screen.ssao.id);
    rlUnloadShaderProgram(R3D.shader.screen.lighting.id);
    rlUnloadShaderProgram(R3D.shader.screen.bloom.id);
    rlUnloadShaderProgram(R3D.shader.screen.fog.id);
    rlUnloadShaderProgram(R3D.shader.screen.tonemap.id);
    rlUnloadShaderProgram(R3D.shader.screen.adjustment.id);
    rlUnloadShaderProgram(R3D.shader.screen.fxaa.id);

    rlUnloadTexture(R3D.texture.white);
    rlUnloadTexture(R3D.texture.black);
    rlUnloadTexture(R3D.texture.normal);
    rlUnloadTexture(R3D.texture.ssaoNoise);
    rlUnloadTexture(R3D.texture.ssaoKernel);
    rlUnloadTexture(R3D.texture.iblBrdfLut);

    r3d_primitive_unload(&R3D.primitive.quad);
    r3d_primitive_unload(&R3D.primitive.cube);
}

bool R3D_HasState(unsigned int flag)
{
    return R3D.state.flags & flag;
}

void R3D_SetState(unsigned int flags)
{
    R3D.state.flags |= flags;
}

void R3D_ClearState(unsigned int flags)
{
    R3D.state.flags &= ~flags;
}

void R3D_GetFXAAParameters(float* qualityLevel, float* edgeSensitivity, float* subpixelQuality)
{
    if (qualityLevel) *qualityLevel = R3D.state.fxaa.qualityLevel;
    if (edgeSensitivity) *edgeSensitivity = R3D.state.fxaa.edgeSensitivity;
    if (subpixelQuality) *subpixelQuality = R3D.state.fxaa.subpixelQuality;
}
void R3D_SetFXAAParameters(float qualityLevel, float edgeSensitivity, float subpixelQuality)
{
    R3D.state.fxaa.qualityLevel = qualityLevel;
    R3D.state.fxaa.edgeSensitivity = edgeSensitivity;
    R3D.state.fxaa.subpixelQuality = subpixelQuality;
}

void R3D_GetResolution(int* width, int* height)
{
    if (width) *width = R3D.state.resolution.width;
    if (height) *height = R3D.state.resolution.height;
}

void R3D_UpdateResolution(int width, int height)
{
    if (width <= 0 || height <= 0) {
        TraceLog(LOG_ERROR, "Invalid resolution given to 'R3D_UpdateResolution'");
        return;
    }

    if (width == R3D.state.resolution.width && height == R3D.state.resolution.height) {
        return;
    }

    r3d_framebuffers_unload();
    r3d_framebuffers_load(width, height);

    R3D.state.resolution.width = width;
    R3D.state.resolution.height = height;
    R3D.state.resolution.texelX = 1.0f / width;
    R3D.state.resolution.texelY = 1.0f / height;
}

void R3D_EnableCustomTarget(RenderTexture target)
{
    R3D.framebuffer.customTarget = target;
}

void R3D_DisableCustomTarget(void)
{
    memset(&R3D.framebuffer.customTarget, 0, sizeof(RenderTexture));
}

void R3D_Begin(Camera3D camera)
{
    // Render the batch before proceeding
    rlDrawRenderBatchActive();

    // Clear the previous draw call array state
    r3d_array_clear(&R3D.container.drawCallArray);

    // Store camera position
    R3D.state.transform.position = camera.position;

    // Compute aspect ratio
    float aspect = 1.0f;
    if (R3D.state.flags & R3D_FLAG_ASPECT_KEEP) {
        aspect = (float)R3D.state.resolution.width / R3D.state.resolution.height;
    }
    else {
        aspect = (float)GetScreenWidth() / GetScreenHeight();
    }

    // Compute projection matrix
    if (camera.projection == CAMERA_PERSPECTIVE) {
        double top = rlGetCullDistanceNear() * tan(camera.fovy * 0.5 * DEG2RAD);
        double right = top * aspect;
        R3D.state.transform.proj = MatrixFrustum(
            -right, right, -top, top,
            rlGetCullDistanceNear(),
            rlGetCullDistanceFar()
        );
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
        double top = camera.fovy / 2.0;
        double right = top * aspect;
        R3D.state.transform.proj = MatrixOrtho(
            -right, right, -top, top,
            rlGetCullDistanceNear(),
            rlGetCullDistanceFar()
        );
    }

    // Compute view matrix
    R3D.state.transform.view = MatrixLookAt(
        camera.position,
        camera.target,
        camera.up
    );

    // Store inverse matrices
    R3D.state.transform.invProj = MatrixInvert(R3D.state.transform.proj);
    R3D.state.transform.invView = MatrixInvert(R3D.state.transform.view);

    // Compute frustum
    Matrix matMV = MatrixMultiply(R3D.state.transform.view, R3D.state.transform.proj);
    R3D.state.frustum.aabb = r3d_frustum_get_bounding_box(matMV);
    R3D.state.frustum.shape = r3d_frustum_create(matMV);
}

void R3D_End(void)
{
    // [PART 1] - Init global state
    {
        rlDisableColorBlend();
    }

    // [PART 2] - Sort draw calls from front to back based on the camera position
    {
        r3d_drawcall_sort_front_to_back(
            R3D.container.drawCallArray.data,
            R3D.container.drawCallArray.count
        );
    }

    // [PART 3] - Raster geometries to the geometry buffers
    {
        rlEnableFramebuffer(R3D.framebuffer.gBuffer.id);
        rlViewport(0, 0, R3D.state.resolution.width, R3D.state.resolution.height);
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
            rlSetMatrixProjection(R3D.state.transform.proj);

            // Setup view matrix
            rlMatrixMode(RL_MODELVIEW);
            rlLoadIdentity();
            rlMultMatrixf(MatrixToFloat(R3D.state.transform.view));

            // Render skybox - (albedo buffer only)
            if (R3D.env.useSky)
            {
                r3d_shader_enable(raster.skybox);
                rlDisableBackfaceCulling();
                rlDisableDepthMask();

                Matrix matView = rlGetMatrixModelview();
                Matrix matProj = rlGetMatrixProjection();

                // Bind cubemap texture
                r3d_shader_bind_samplerCube(raster.skybox, uCubeSky, R3D.env.sky.cubemap.id);

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
                r3d_shader_unbind_samplerCube(raster.skybox, uCubeSky);

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
                        r3d_drawcall_raster_geometry_material(
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

    // [PART 4] - Process SSAO
    if (R3D.env.ssaoEnabled) {
        rlEnableFramebuffer(R3D.framebuffer.pingPongSSAO.id);
        rlViewport(0, 0, R3D.state.resolution.width / 2, R3D.state.resolution.height / 2);
        {
            // Bind first SSAO output texture
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                R3D.framebuffer.pingPongSSAO.textures[0], 0
            );

            // Render SSAO
            r3d_shader_enable(screen.ssao);
            {
                r3d_shader_set_mat4(screen.ssao, uMatInvProj, R3D.state.transform.invProj);
                r3d_shader_set_mat4(screen.ssao, uMatInvView, R3D.state.transform.invView);
                r3d_shader_set_mat4(screen.ssao, uMatProj, R3D.state.transform.proj);
                r3d_shader_set_mat4(screen.ssao, uMatView, R3D.state.transform.view);

                r3d_shader_set_vec2(screen.ssao, uResolution, ((Vector2) {
                    R3D.state.resolution.width / 2, R3D.state.resolution.height / 2
                }));

                r3d_shader_set_float(screen.ssao, uNear, rlGetCullDistanceNear());
                r3d_shader_set_float(screen.ssao, uFar, rlGetCullDistanceFar());

                r3d_shader_set_float(screen.ssao, uRadius, R3D.env.ssaoRadius);
                r3d_shader_set_float(screen.ssao, uBias, R3D.env.ssaoBias);

                r3d_shader_bind_sampler2D(screen.ssao, uTexDepth, R3D.framebuffer.gBuffer.depth);
                r3d_shader_bind_sampler2D(screen.ssao, uTexNormal, R3D.framebuffer.gBuffer.normal);
                r3d_shader_bind_sampler1D(screen.ssao, uTexKernel, R3D.texture.ssaoKernel);
                r3d_shader_bind_sampler2D(screen.ssao, uTexNoise, R3D.texture.ssaoNoise);

                r3d_primitive_draw_quad();

                r3d_shader_unbind_sampler2D(screen.ssao, uTexDepth);
                r3d_shader_unbind_sampler2D(screen.ssao, uTexNormal);
                r3d_shader_unbind_sampler1D(screen.ssao, uTexKernel);
                r3d_shader_unbind_sampler2D(screen.ssao, uTexNoise);
            }
            r3d_shader_disable();

            // Blur SSAO
            {
                bool* horizontalPass = &R3D.framebuffer.pingPongSSAO.targetTextureIdx;
                *horizontalPass = true;

                r3d_shader_enable(generate.gaussianBlurDualPass)
                {
                    for (int i = 0; i < R3D.env.ssaoIterations; i++, (*horizontalPass) = !(*horizontalPass)) {
                        glFramebufferTexture2D(
                            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            R3D.framebuffer.pingPongSSAO.textures[(*horizontalPass)], 0
                        );
                        r3d_shader_set_vec2(generate.gaussianBlurDualPass, uDirection,
                            ((*horizontalPass) ? (Vector2) { 1, 0 } : (Vector2) { 0, 1 })
                        );
                        r3d_shader_bind_sampler2D(generate.gaussianBlurDualPass, uTexture,
                            R3D.framebuffer.pingPongSSAO.textures[!(*horizontalPass)]
                        );
                        r3d_primitive_draw_quad();
                    }
                }
                r3d_shader_disable();
            }
        }
        rlDisableFramebuffer();
    }

    // [PART 5] - Determine what light should lit the visible scene
    r3d_light_t* lights[8] = { 0 };
    int lightCount = 0;
    {
        for (int id = 1; id <= r3d_registry_get_allocated_count(&R3D.container.lightRegistry); id++) {
            if (!r3d_registry_is_valid(&R3D.container.lightRegistry, id)) continue;
            r3d_light_t* light = r3d_registry_get(&R3D.container.lightRegistry, id);
            if (!light->enabled) continue;
            lights[lightCount++] = light;
        }
    }

    // [PART 6] - Render shadow maps for each light
    {
        rlEnableDepthTest();

        for (int i = 0; i < lightCount; i++) {
            r3d_light_t* light = lights[i];

            // Skip light if it doesn't produce shadows
            if (!light->shadow.enabled) continue;

            // Start rendering to shadow map
            rlEnableFramebuffer(light->shadow.map.id);
            rlViewport(0, 0, light->shadow.map.resolution, light->shadow.map.resolution);
            {
                // Set shader
                r3d_shader_enable(raster.depthCube);

                if (light->type == R3D_LIGHT_OMNI) {
                    // Set up projection matrix for omni-directional light
                    rlMatrixMode(RL_PROJECTION);
                    rlSetMatrixProjection(MatrixPerspective(90 * DEG2RAD, 1.0, 0.05, light->range));

                    // Render geometries for each face of the cubemap
                    for (int j = 0; j < 6; j++) {
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, light->shadow.map.color, 0);
                        glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                        // Set view matrix for the current cubemap face
                        rlMatrixMode(RL_MODELVIEW);
                        rlLoadIdentity();
                        rlMultMatrixf(MatrixToFloat(r3d_light_get_matrix_view_omni(light, j)));

                        // Rasterize geometries for depth rendering
                        for (int i = 0; i < R3D.container.drawCallArray.count; i++) {
                            r3d_drawcall_raster_geometry_depth_cube(
                                (r3d_drawcall_t*)R3D.container.drawCallArray.data + i,
                                light->position
                            );
                        }
                    }
                }
                else {
                    // Clear depth buffer for other light types
                    glClear(GL_DEPTH_BUFFER_BIT);

                    // Set up shader for directional or spot light
                    r3d_shader_enable(raster.depth);

                    Matrix matView = { 0 };
                    Matrix matProj = { 0 };

                    if (light->type == R3D_LIGHT_DIR) {
                        Vector3 lightPos = Vector3Add((Vector3) { 0 }, Vector3Scale(Vector3Negate(light->direction), 1000.0));
                        matView = MatrixLookAt(lightPos, (Vector3) { 0 }, (Vector3) { 0, 1, 0 });
                        matProj = MatrixOrtho(-10, 10, -10, 10, 0.1, 2000.0);
                    }
                    else if (light->type == R3D_LIGHT_SPOT) {
                        matView = MatrixLookAt(light->position, Vector3Add(light->position, light->direction), (Vector3) { 0, 1, 0 });
                        matProj = MatrixPerspective(90 * DEG2RAD, 1.0, 0.05, light->range);
                    }

                    // Store combined view and projection matrix for the shadow map
                    light->shadow.matViewProj = MatrixMultiply(matView, matProj);

                    // Set projection and view matrices
                    rlMatrixMode(RL_PROJECTION);
                    rlSetMatrixProjection(matProj);
                    rlMatrixMode(RL_MODELVIEW);
                    rlLoadIdentity();
                    rlMultMatrixf(MatrixToFloat(matView));

                    // Rasterize geometry for depth rendering
                    for (int i = 0; i < R3D.container.drawCallArray.count; i++) {
                        r3d_drawcall_raster_geometry_depth(
                            (r3d_drawcall_t*)R3D.container.drawCallArray.data + i
                        );
                    }
                }

                r3d_shader_disable();
            }
            rlDisableFramebuffer();
        }

        rlDisableDepthTest();
    }

    // [PART 7] - Lighting computation from G-buffer data into the final render target
    {
        rlEnableFramebuffer(R3D.framebuffer.lit.id);
        rlViewport(0, 0, R3D.state.resolution.width, R3D.state.resolution.height);
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

                    // Send shadow map data
                    if (light->shadow.enabled) {
                        if (light->type == R3D_LIGHT_OMNI) {
                            r3d_shader_bind_samplerCube(screen.lighting, uLights[i].shadowCubemap, light->shadow.map.color);
                        }
                        else {
                            r3d_shader_set_float(screen.lighting, uLights[i].shadowMapTxlSz, light->shadow.map.texelSize);
                            r3d_shader_bind_sampler2D(screen.lighting, uLights[i].shadowMap, light->shadow.map.depth);
                            r3d_shader_set_mat4(screen.lighting, uLights[i].matViewProj, light->shadow.matViewProj);
                        }
                        r3d_shader_set_float(screen.lighting, uLights[i].shadowBias, light->shadow.bias);
                        r3d_shader_set_int(screen.lighting, uLights[i].shadow, true);
                    }
                    else {
                        r3d_shader_set_int(screen.lighting, uLights[i].shadow, false);
                    }
                }

                for (int i = lightCount; i < R3D_SHADER_NUM_LIGHTS; i++) {
                    r3d_shader_set_int(screen.lighting, uLights[i].enabled, false);
                }

                if (R3D.env.useSky) {
                    r3d_shader_bind_samplerCube(screen.lighting, uCubeIrradiance, R3D.env.sky.irradiance.id);
                    r3d_shader_bind_samplerCube(screen.lighting, uCubePrefilter, R3D.env.sky.prefilter.id);
                    r3d_shader_bind_sampler2D(screen.lighting, uTexBrdfLut, R3D.texture.iblBrdfLut);

                    r3d_shader_set_vec4(screen.lighting, uQuatSkybox, R3D.env.quatSky);
                    r3d_shader_set_int(screen.lighting, uHasSkybox, true);
                }
                else {
                    r3d_shader_set_vec3(screen.lighting, uColAmbient, R3D.env.ambientColor);
                    r3d_shader_set_int(screen.lighting, uHasSkybox, false);
                }

                r3d_shader_set_mat4(screen.lighting, uMatInvProj, R3D.state.transform.invProj);
                r3d_shader_set_mat4(screen.lighting, uMatInvView, R3D.state.transform.invView);
                r3d_shader_set_vec3(screen.lighting, uViewPosition, R3D.state.transform.position);

                r3d_shader_set_float(screen.lighting, uBloomHdrThreshold, R3D.env.bloomHdrThreshold);

                r3d_shader_bind_sampler2D(screen.lighting, uTexAlbedo, R3D.framebuffer.gBuffer.albedo);
                r3d_shader_bind_sampler2D(screen.lighting, uTexEmission, R3D.framebuffer.gBuffer.emission);
                r3d_shader_bind_sampler2D(screen.lighting, uTexNormal, R3D.framebuffer.gBuffer.normal);
                r3d_shader_bind_sampler2D(screen.lighting, uTexDepth, R3D.framebuffer.gBuffer.depth);
                if (R3D.env.ssaoEnabled) {
                    r3d_shader_bind_sampler2D(screen.lighting, uTexSSAO,
                        R3D.framebuffer.pingPongSSAO.textures[!R3D.framebuffer.pingPongSSAO.targetTextureIdx]
                    );
                }
                else {
                    r3d_shader_bind_sampler2D(screen.lighting, uTexSSAO, R3D.texture.white);
                }
                r3d_shader_bind_sampler2D(screen.lighting, uTexORM, R3D.framebuffer.gBuffer.orm);
                r3d_shader_bind_sampler2D(screen.lighting, uTexID, R3D.framebuffer.gBuffer.matId);

                r3d_primitive_draw_quad();

                r3d_shader_unbind_sampler2D(screen.lighting, uTexAlbedo);
                r3d_shader_unbind_sampler2D(screen.lighting, uTexEmission);
                r3d_shader_unbind_sampler2D(screen.lighting, uTexNormal);
                r3d_shader_unbind_sampler2D(screen.lighting, uTexDepth);
                r3d_shader_unbind_sampler2D(screen.lighting, uTexSSAO);
                r3d_shader_unbind_sampler2D(screen.lighting, uTexORM);
                r3d_shader_unbind_sampler2D(screen.lighting, uTexID);

                if (R3D.env.useSky) {
                    r3d_shader_unbind_samplerCube(screen.lighting, uCubeIrradiance);
                    r3d_shader_unbind_samplerCube(screen.lighting, uCubePrefilter);
                    r3d_shader_unbind_sampler2D(screen.lighting, uTexBrdfLut);
                }

                for (int i = 0; i < lightCount; i++) {
                    r3d_light_t* light = lights[i];
                    if (light->shadow.enabled) {
                        if (light->type == R3D_LIGHT_OMNI) {
                            r3d_shader_unbind_samplerCube(screen.lighting, uLights[i].shadowCubemap);
                        }
                        else {
                            r3d_shader_unbind_sampler2D(screen.lighting, uLights[i].shadowMap);
                        }
                    }
                }
            }
            r3d_shader_disable();
        }
        rlDisableFramebuffer();
    }

    // [PART 8] - Post proccesses using ping-pong buffer
    {
        // Generating the blur for the brightness buffer to create the bloom effect.
        if (R3D.env.bloomMode != R3D_BLOOM_DISABLED) {
            rlEnableFramebuffer(R3D.framebuffer.pingPongBloom.id);
            rlViewport(0, 0, R3D.state.resolution.width / 2, R3D.state.resolution.height / 2);
            {
                bool* horizontalPass = &R3D.framebuffer.pingPongBloom.targetTextureIdx;
                *horizontalPass = true;

                r3d_shader_enable(generate.gaussianBlurDualPass)
                {
                    for (int i = 0; i < R3D.env.bloomIterations; i++, (*horizontalPass) = !(*horizontalPass)) {
                        glFramebufferTexture2D(
                            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                            R3D.framebuffer.pingPongBloom.textures[(*horizontalPass)], 0
                        );
                        r3d_shader_set_vec2(generate.gaussianBlurDualPass, uDirection,
                            ((*horizontalPass) ? (Vector2) { 1, 0 } : (Vector2) { 0, 1 })
                        );
                        r3d_shader_bind_sampler2D(generate.gaussianBlurDualPass, uTexture, i > 0
                            ? R3D.framebuffer.pingPongBloom.textures[!(*horizontalPass)]
                            : R3D.framebuffer.lit.bright
                        );
                        r3d_primitive_draw_quad();
                    }
                }
                r3d_shader_disable();
            }
            rlDisableFramebuffer();
        }

        // Initializing data to alternate between the
        // source/destination textures of the post-effects framebuffer.
        int texIndex = 2;
        unsigned int textures[3] = {
            R3D.framebuffer.post.textures[0],
            R3D.framebuffer.post.textures[1],
            R3D.framebuffer.lit.color
        };

        // Post effect rendering
        rlEnableFramebuffer(R3D.framebuffer.post.id);
        rlViewport(0, 0, R3D.state.resolution.width, R3D.state.resolution.height);
        {
            // Post process: Bloom
            if (R3D.env.bloomMode != R3D_BLOOM_DISABLED) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, textures[!texIndex], 0
                );
                r3d_shader_enable(screen.bloom);
                {
                    r3d_shader_bind_sampler2D(screen.bloom, uTexColor, textures[texIndex]);
                    r3d_shader_bind_sampler2D(screen.bloom, uTexBloomBlur,
                        R3D.framebuffer.pingPongBloom.textures[
                            !R3D.framebuffer.pingPongBloom.targetTextureIdx
                        ]
                    );
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
                    r3d_shader_bind_sampler2D(screen.fog, uTexColor, textures[texIndex]);
                    r3d_shader_bind_sampler2D(screen.fog, uTexDepth, R3D.framebuffer.gBuffer.depth);
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
                    r3d_shader_bind_sampler2D(screen.tonemap, uTexColor, textures[texIndex]);
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
                    r3d_shader_bind_sampler2D(screen.adjustment, uTexColor, textures[texIndex]);
                    texIndex = !texIndex;

                    r3d_shader_set_float(screen.adjustment, uBrightness, R3D.env.brightness);
                    r3d_shader_set_float(screen.adjustment, uContrast, R3D.env.contrast);
                    r3d_shader_set_float(screen.adjustment, uSaturation, R3D.env.saturation);

                    r3d_primitive_draw_quad();
                }
                r3d_shader_disable();
            }

            // Post process: Anti aliasing
            if (R3D.state.flags & R3D_FLAG_FXAA) {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D, textures[!texIndex], 0
                );
                r3d_shader_enable(screen.fxaa);
                {
                    r3d_shader_bind_sampler2D(screen.fxaa, uTexture, textures[texIndex]);
                    texIndex = !texIndex;

                    Vector2 texelSize = { R3D.state.resolution.texelX, R3D.state.resolution.texelY };
                    r3d_shader_set_vec2(screen.fxaa, uTexelSize, texelSize);

                    r3d_shader_set_float(screen.fxaa, uQualityLevel, R3D.state.fxaa.qualityLevel);
                    r3d_shader_set_float(screen.fxaa, uEdgeSensitivity, R3D.state.fxaa.edgeSensitivity);
                    r3d_shader_set_float(screen.fxaa, uSubpixelQuality, R3D.state.fxaa.subpixelQuality);

                    r3d_primitive_draw_quad();
                }
                r3d_shader_disable();
            }
        }
        rlDisableFramebuffer();
    }

    // [PART 9] - Blit the final result to the main framebuffer
    {
        unsigned int dstId = 0;
        int dstX = 0, dstY = 0;
        int dstW = GetScreenWidth();
        int dstH = GetScreenHeight();

        // If a custom final framebuffer is set, use its ID and dimensions
        if (R3D.framebuffer.customTarget.id != 0) {
            dstId = R3D.framebuffer.customTarget.id;
            dstW = R3D.framebuffer.customTarget.texture.width;
            dstH = R3D.framebuffer.customTarget.texture.height;
        }

        // Maintain aspect ratio if the corresponding flag is set
        if (R3D.state.flags & R3D_FLAG_ASPECT_KEEP) {
            float srcRatio = (float)R3D.state.resolution.width / R3D.state.resolution.height;
            float dstRatio = (float)dstW / dstH;
            if (srcRatio > dstRatio) {
                int prevH = dstH;
                dstH = dstW * srcRatio;
                dstY = (prevH - dstH) / 2;
            }
            else {
                int prevW = dstW;
                dstW = dstH * srcRatio;
                dstX = (prevW - dstW) / 2;
            }
        }

        // Bind the destination framebuffer for drawing
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstId);

        // Blit only the color data from the post-processing framebuffer to the main framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, R3D.framebuffer.post.id);
        glBlitFramebuffer(
            0, 0, R3D.state.resolution.width, R3D.state.resolution.height,
            dstX, dstY, dstX + dstW, dstY + dstH, GL_COLOR_BUFFER_BIT,
            (R3D.state.flags & R3D_FLAG_BLIT_LINEAR) ? GL_LINEAR : GL_NEAREST
        );

        // Blit the depth data from the gbuffer framebuffer to the main framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, R3D.framebuffer.gBuffer.id);
        glBlitFramebuffer(
            0, 0, R3D.state.resolution.width, R3D.state.resolution.height,
            dstX, dstY, dstX + dstW, dstY + dstH,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
    }

    // [PART 10] - Reset global state
    {
        rlViewport(0, 0, GetRenderWidth(), GetRenderHeight());
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


/* === Internal functions === */

static void r3d_framebuffers_load(int width, int height)
{
    r3d_framebuffer_load_gbuffer(width, height);
    r3d_framebuffer_load_lit(width, height);
    r3d_framebuffer_load_post(width, height);

    if (R3D.env.ssaoEnabled) {
        r3d_framebuffer_load_pingpong_ssao(width, height);
    }

    if (R3D.env.bloomMode != R3D_BLOOM_DISABLED) {
        r3d_framebuffer_load_pingpong_bloom(width, height);
    }
}

static void r3d_framebuffers_unload(void)
{
    r3d_framebuffer_unload_gbuffer();
    r3d_framebuffer_unload_lit();
    r3d_framebuffer_unload_post();

    if (R3D.framebuffer.pingPongSSAO.id != 0) {
        r3d_framebuffer_unload_pingpong_ssao();
    }

    if (R3D.framebuffer.pingPongBloom.id != 0) {
        r3d_framebuffer_unload_pingpong_bloom();
    }
}
