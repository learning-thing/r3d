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

#include "./r3d_state.h"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <glad.h>

#include "./embedded/r3d_shaders.h"
#include "./embedded/r3d_textures.h"
#include "./details/misc/r3d_half.h"
#include "./details/r3d_dds_loader_ext.h"


/* === Global state definition === */

struct R3D_State R3D = { 0 };


/* === Framebuffer loading functions === */

void r3d_framebuffer_load_gbuffer(int width, int height)
{
    struct r3d_fb_gbuffer_t* gBuffer = &R3D.framebuffer.gBuffer;

    gBuffer->id = rlLoadFramebuffer();
    if (gBuffer->id == 0) {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
    }

    rlEnableFramebuffer(gBuffer->id);

    // Generate (albedo / orm / emission / material ID) buffers
    gBuffer->albedo = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);
    gBuffer->emission = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16, 1);
    gBuffer->orm = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);

    // We generate the normal buffer here.
    // The setup for the normal buffer requires direct API calls
    // since RLGL does not support the creation of 16-bit two-component textures.
    // Normals will be encoded and decoded using octahedral mapping for efficient storage and reconstruction.
    glGenTextures(1, &gBuffer->normal);
    glBindTexture(GL_TEXTURE_2D, gBuffer->normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate depth render buffer
    gBuffer->depth = rlLoadTextureDepth(width, height, false);

    // Activate the draw buffers for all the attachments
    rlActiveDrawBuffers(R3D_GBUFFER_COUNT);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(gBuffer->id, gBuffer->albedo, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->emission, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->normal, RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->orm, RL_ATTACHMENT_COLOR_CHANNEL3, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->depth, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(gBuffer->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }
}

void r3d_framebuffer_load_pingpong_ssao(int width, int height)
{
    struct r3d_fb_pingpong_ssao_t* ssao = &R3D.framebuffer.pingPongSSAO;

    width /= 2, height /= 2;

    ssao->id = rlLoadFramebuffer();
    if (ssao->id == 0) {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
    }

    rlEnableFramebuffer(ssao->id);

    // Generate (ssao) buffers
    for (int i = 0; i < 2; i++) {
        glGenTextures(1, &ssao->textures[i]);
        glBindTexture(GL_TEXTURE_2D, ssao->textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Activate the draw buffers for all the attachments
    rlActiveDrawBuffers(1);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(ssao->id, ssao->textures[0], RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(ssao->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }

    // Internal data setup
    ssao->targetTexIdx = 0;
}

void r3d_framebuffer_load_lit(int width, int height)
{
    struct r3d_fb_lit_t* lit = &R3D.framebuffer.lit;

    lit->id = rlLoadFramebuffer();
    if (lit->id == 0) {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
    }

    rlEnableFramebuffer(lit->id);

    // Generate (color / luminance) buffers
    lit->color = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);
    lit->bright = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);

    // Setup color texture parameters
    glBindTexture(GL_TEXTURE_2D, lit->color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Setup bright texture parameters
    glBindTexture(GL_TEXTURE_2D, lit->bright);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Activate the draw buffers for all the attachments
    rlActiveDrawBuffers(2);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(lit->id, lit->color, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(lit->id, lit->bright, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(lit->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }
}

void r3d_framebuffer_load_pingpong_bloom(int width, int height)
{
    struct r3d_fb_pingpong_bloom_t* bloom = &R3D.framebuffer.pingPongBloom;

    width /= 2, height /= 2;

    bloom->id = rlLoadFramebuffer();
    if (bloom->id == 0) {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
    }

    rlEnableFramebuffer(bloom->id);

    // Generate (color) buffers
    for (int i = 0; i < 2; i++) {
        bloom->textures[i] = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);
        glBindTexture(GL_TEXTURE_2D, bloom->textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Activate the draw buffers for all the attachments
    rlActiveDrawBuffers(1);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(bloom->id, bloom->textures[0], RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(bloom->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }

    // Internal data setup
    bloom->targetTexIdx = 0;
}

void r3d_framebuffer_load_post(int width, int height)
{
    struct r3d_fb_post_t* post = &R3D.framebuffer.post;

    post->id = rlLoadFramebuffer();
    if (post->id == 0) {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
    }

    rlEnableFramebuffer(post->id);

    // Generate (color) buffers
    post->textures[0] = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);
    post->textures[1] = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);

    // Activate the draw buffers for all the attachments
    rlActiveDrawBuffers(1);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(post->id, post->textures[0], RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(post->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }

    // Internal data setup
    post->targetTexIdx = 0;
}

void r3d_framebuffer_unload_gbuffer(void)
{
    struct r3d_fb_gbuffer_t* gBuffer = &R3D.framebuffer.gBuffer;

    rlUnloadTexture(gBuffer->albedo);
    rlUnloadTexture(gBuffer->emission);
    rlUnloadTexture(gBuffer->normal);
    rlUnloadTexture(gBuffer->orm);
    rlUnloadTexture(gBuffer->depth);

    rlUnloadFramebuffer(gBuffer->id);

    memset(gBuffer, 0, sizeof(struct r3d_fb_gbuffer_t));
}

void r3d_framebuffer_unload_pingpong_ssao(void)
{
    struct r3d_fb_pingpong_ssao_t* ssao = &R3D.framebuffer.pingPongSSAO;

    for (int i = 0; i < sizeof(ssao->textures) / sizeof(*ssao->textures); i++) {
        rlUnloadTexture(ssao->textures[i]);
    }

    rlUnloadFramebuffer(ssao->id);

    memset(ssao, 0, sizeof(struct r3d_fb_pingpong_ssao_t));
}

void r3d_framebuffer_unload_lit(void)
{
    struct r3d_fb_lit_t* lit = &R3D.framebuffer.lit;

    rlUnloadTexture(lit->color);
    rlUnloadTexture(lit->bright);

    rlUnloadFramebuffer(lit->id);

    memset(lit, 0, sizeof(struct r3d_fb_lit_t));
}

void r3d_framebuffer_unload_pingpong_bloom(void)
{
    struct r3d_fb_pingpong_bloom_t* bloom = &R3D.framebuffer.pingPongBloom;

    for (int i = 0; i < sizeof(bloom->textures) / sizeof(*bloom->textures); i++) {
        rlUnloadTexture(bloom->textures[i]);
    }

    rlUnloadFramebuffer(bloom->id);

    memset(bloom, 0, sizeof(struct r3d_fb_pingpong_bloom_t));
}

void r3d_framebuffer_unload_post(void)
{
    struct r3d_fb_post_t* post = &R3D.framebuffer.post;

    for (int i = 0; i < sizeof(post->textures) / sizeof(*post->textures); i++) {
        rlUnloadTexture(post->textures[i]);
    }

    rlUnloadFramebuffer(post->id);

    memset(post, 0, sizeof(struct r3d_fb_post_t));
}


/* === Shader loading functions === */

void r3d_shader_load_generate_gaussian_blur_dual_pass(void)
{
    R3D.shader.generate.gaussianBlurDualPass.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_GENERATE_GAUSSIAN_BLUR_DUAL_PASS
    );

    r3d_shader_get_location(generate.gaussianBlurDualPass, uTexture);
    r3d_shader_get_location(generate.gaussianBlurDualPass, uTexelDir);

    r3d_shader_enable(generate.gaussianBlurDualPass);
    r3d_shader_set_sampler2D_slot(generate.gaussianBlurDualPass, uTexture, 0);
    r3d_shader_disable();
}

void r3d_shader_load_generate_cubemap_from_equirectangular(void)
{
    R3D.shader.generate.cubemapFromEquirectangular.id = rlLoadShaderCode(
        VS_COMMON_CUBEMAP, FS_GENERATE_CUBEMAP_FROM_EQUIRECTANGULAR
    );

    r3d_shader_get_location(generate.cubemapFromEquirectangular, uMatProj);
    r3d_shader_get_location(generate.cubemapFromEquirectangular, uMatView);
    r3d_shader_get_location(generate.cubemapFromEquirectangular, uTexEquirectangular);

    r3d_shader_enable(generate.cubemapFromEquirectangular);
    r3d_shader_set_sampler2D_slot(generate.cubemapFromEquirectangular, uTexEquirectangular, 0);
    r3d_shader_disable();
}

void r3d_shader_load_generate_irradiance_convolution(void)
{
    R3D.shader.generate.irradianceConvolution.id = rlLoadShaderCode(
        VS_COMMON_CUBEMAP, FS_GENERATE_IRRADIANCE_CONVOLUTION
    );

    r3d_shader_get_location(generate.irradianceConvolution, uMatProj);
    r3d_shader_get_location(generate.irradianceConvolution, uMatView);
    r3d_shader_get_location(generate.irradianceConvolution, uCubemap);

    r3d_shader_enable(generate.irradianceConvolution);
    r3d_shader_set_samplerCube_slot(generate.irradianceConvolution, uCubemap, 0);
    r3d_shader_disable();
}

void r3d_shader_load_generate_prefilter(void)
{
    R3D.shader.generate.prefilter.id = rlLoadShaderCode(
        VS_COMMON_CUBEMAP, FS_GENERATE_PREFILTER
    );

    r3d_shader_get_location(generate.prefilter, uMatProj);
    r3d_shader_get_location(generate.prefilter, uMatView);
    r3d_shader_get_location(generate.prefilter, uCubemap);
    r3d_shader_get_location(generate.prefilter, uRoughness);

    r3d_shader_enable(generate.prefilter);
    r3d_shader_set_samplerCube_slot(generate.prefilter, uCubemap, 0);
    r3d_shader_disable();
}

void r3d_shader_load_raster_geometry(void)
{
    R3D.shader.raster.geometry.id = rlLoadShaderCode(
        VS_RASTER_GEOMETRY, FS_RASTER_GEOMETRY
    );

    r3d_shader_get_location(raster.geometry, uMatNormal);
    r3d_shader_get_location(raster.geometry, uMatModel);
    r3d_shader_get_location(raster.geometry, uMatMVP);
    r3d_shader_get_location(raster.geometry, uTexAlbedo);
    r3d_shader_get_location(raster.geometry, uTexNormal);
    r3d_shader_get_location(raster.geometry, uTexEmission);
    r3d_shader_get_location(raster.geometry, uTexOcclusion);
    r3d_shader_get_location(raster.geometry, uTexRoughness);
    r3d_shader_get_location(raster.geometry, uTexMetalness);
    r3d_shader_get_location(raster.geometry, uValEmission);
    r3d_shader_get_location(raster.geometry, uValOcclusion);
    r3d_shader_get_location(raster.geometry, uValRoughness);
    r3d_shader_get_location(raster.geometry, uValMetalness);
    r3d_shader_get_location(raster.geometry, uColAlbedo);
    r3d_shader_get_location(raster.geometry, uColEmission);

    r3d_shader_enable(raster.geometry);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexAlbedo, 0);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexNormal, 1);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexEmission, 2);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexOcclusion, 3);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexRoughness, 4);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexMetalness, 5);
    r3d_shader_disable();
}

void r3d_shader_load_raster_geometry_inst(void)
{
    R3D.shader.raster.geometryInst.id = rlLoadShaderCode(
        VS_RASTER_GEOMETRY_INST, FS_RASTER_GEOMETRY
    );

    r3d_shader_get_location(raster.geometry, uMatModel);
    r3d_shader_get_location(raster.geometry, uMatMVP);
    r3d_shader_get_location(raster.geometry, uTexAlbedo);
    r3d_shader_get_location(raster.geometry, uTexNormal);
    r3d_shader_get_location(raster.geometry, uTexEmission);
    r3d_shader_get_location(raster.geometry, uTexOcclusion);
    r3d_shader_get_location(raster.geometry, uTexRoughness);
    r3d_shader_get_location(raster.geometry, uTexMetalness);
    r3d_shader_get_location(raster.geometry, uValEmission);
    r3d_shader_get_location(raster.geometry, uValOcclusion);
    r3d_shader_get_location(raster.geometry, uValRoughness);
    r3d_shader_get_location(raster.geometry, uValMetalness);
    r3d_shader_get_location(raster.geometry, uColAlbedo);
    r3d_shader_get_location(raster.geometry, uColEmission);

    r3d_shader_enable(raster.geometry);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexAlbedo, 0);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexNormal, 1);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexEmission, 2);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexOcclusion, 3);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexRoughness, 4);
    r3d_shader_set_sampler2D_slot(raster.geometry, uTexMetalness, 5);
    r3d_shader_disable();
}

void r3d_shader_load_raster_skybox(void)
{
    R3D.shader.raster.skybox.id = rlLoadShaderCode(
        VS_RASTER_SKYBOX, FS_RASTER_SKYBOX
    );

    r3d_shader_get_location(raster.skybox, uMatProj);
    r3d_shader_get_location(raster.skybox, uMatView);
    r3d_shader_get_location(raster.skybox, uRotation);
    r3d_shader_get_location(raster.skybox, uCubeSky);

    r3d_shader_enable(raster.skybox);
    r3d_shader_set_samplerCube_slot(raster.skybox, uCubeSky, 0);
    r3d_shader_disable();
}

void r3d_shader_load_raster_depth(void)
{
    R3D.shader.raster.depth.id = rlLoadShaderCode(
        VS_RASTER_DEPTH, FS_RASTER_DEPTH
    );

    r3d_shader_get_location(raster.depth, uMatMVP);
}

void r3d_shader_load_raster_depth_inst(void)
{
    R3D.shader.raster.depthInst.id = rlLoadShaderCode(
        VS_RASTER_DEPTH_INST, FS_RASTER_DEPTH
    );

    r3d_shader_get_location(raster.depth, uMatMVP);
}

void r3d_shader_load_raster_depth_cube(void)
{
    R3D.shader.raster.depthCube.id = rlLoadShaderCode(
        VS_RASTER_DEPTH_CUBE, FS_RASTER_DEPTH_CUBE
    );

    r3d_shader_get_location(raster.depthCube, uViewPosition);
    r3d_shader_get_location(raster.depthCube, uMatModel);
    r3d_shader_get_location(raster.depthCube, uMatMVP);
}

void r3d_shader_load_raster_depth_cube_inst(void)
{
    R3D.shader.raster.depthCubeInst.id = rlLoadShaderCode(
        VS_RASTER_DEPTH_CUBE_INST, FS_RASTER_DEPTH_CUBE
    );

    r3d_shader_get_location(raster.depthCube, uViewPosition);
    r3d_shader_get_location(raster.depthCube, uMatModel);
    r3d_shader_get_location(raster.depthCube, uMatMVP);
}

void r3d_shader_load_screen_ssao(void)
{
    R3D.shader.screen.ssao.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_SSAO
    );

    r3d_shader_get_location(screen.ssao, uTexDepth);
    r3d_shader_get_location(screen.ssao, uTexNormal);
    r3d_shader_get_location(screen.ssao, uTexKernel);
    r3d_shader_get_location(screen.ssao, uTexNoise);
    r3d_shader_get_location(screen.ssao, uMatInvProj);
    r3d_shader_get_location(screen.ssao, uMatInvView);
    r3d_shader_get_location(screen.ssao, uMatProj);
    r3d_shader_get_location(screen.ssao, uMatView);
    r3d_shader_get_location(screen.ssao, uResolution);
    r3d_shader_get_location(screen.ssao, uNear);
    r3d_shader_get_location(screen.ssao, uFar);
    r3d_shader_get_location(screen.ssao, uRadius);
    r3d_shader_get_location(screen.ssao, uBias);

    r3d_shader_enable(screen.ssao);
    r3d_shader_set_sampler2D_slot(screen.ssao, uTexDepth, 0);
    r3d_shader_set_sampler2D_slot(screen.ssao, uTexNormal, 1);
    r3d_shader_set_sampler1D_slot(screen.ssao, uTexKernel, 2);
    r3d_shader_set_sampler2D_slot(screen.ssao, uTexNoise, 3);
    r3d_shader_disable();
}

void r3d_shader_load_screen_lighting(void)
{
    R3D.shader.screen.lighting.id = rlLoadShaderCode(VS_COMMON_SCREEN, FS_SCREEN_LIGHTING);
    r3d_shader_screen_lighting_t* shader = &R3D.shader.screen.lighting;

    r3d_shader_get_location(screen.lighting, uTexAlbedo);
    r3d_shader_get_location(screen.lighting, uTexEmission);
    r3d_shader_get_location(screen.lighting, uTexNormal);
    r3d_shader_get_location(screen.lighting, uTexDepth);
    r3d_shader_get_location(screen.lighting, uTexSSAO);
    r3d_shader_get_location(screen.lighting, uTexORM);
    r3d_shader_get_location(screen.lighting, uColAmbient);
    r3d_shader_get_location(screen.lighting, uCubeIrradiance);
    r3d_shader_get_location(screen.lighting, uCubePrefilter);
    r3d_shader_get_location(screen.lighting, uTexBrdfLut);
    r3d_shader_get_location(screen.lighting, uQuatSkybox);
    r3d_shader_get_location(screen.lighting, uHasSkybox);
    r3d_shader_get_location(screen.lighting, uBloomHdrThreshold);
    r3d_shader_get_location(screen.lighting, uViewPosition);
    r3d_shader_get_location(screen.lighting, uMatInvProj);
    r3d_shader_get_location(screen.lighting, uMatInvView);

    r3d_shader_enable(screen.lighting);

    r3d_shader_set_sampler2D_slot(screen.lighting, uTexAlbedo, 0);
    r3d_shader_set_sampler2D_slot(screen.lighting, uTexEmission, 1);
    r3d_shader_set_sampler2D_slot(screen.lighting, uTexNormal, 2);
    r3d_shader_set_sampler2D_slot(screen.lighting, uTexDepth, 3);
    r3d_shader_set_sampler2D_slot(screen.lighting, uTexSSAO, 4);
    r3d_shader_set_sampler2D_slot(screen.lighting, uTexORM, 5);
    r3d_shader_set_samplerCube_slot(screen.lighting, uCubeIrradiance, 7);
    r3d_shader_set_samplerCube_slot(screen.lighting, uCubePrefilter, 8);
    r3d_shader_set_sampler2D_slot(screen.lighting, uTexBrdfLut, 9);

    int shadowMapSlot = 10;
    for (int i = 0; i < R3D_SHADER_NUM_LIGHTS; i++) {
        shader->uLights[i].matViewProj.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].matViewProj", i));
        shader->uLights[i].shadowMap.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].shadowMap", i));
        shader->uLights[i].shadowCubemap.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].shadowCubemap", i));
        shader->uLights[i].color.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].color", i));
        shader->uLights[i].position.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].position", i));
        shader->uLights[i].direction.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].direction", i));
        shader->uLights[i].energy.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].energy", i));
        shader->uLights[i].range.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].range", i));
        shader->uLights[i].attenuation.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].attenuation", i));
        shader->uLights[i].innerCutOff.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].innerCutOff", i));
        shader->uLights[i].outerCutOff.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].outerCutOff", i));
        shader->uLights[i].shadowMapTxlSz.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].shadowMapTxlSz", i));
        shader->uLights[i].shadowBias.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].shadowBias", i));
        shader->uLights[i].type.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].type", i));
        shader->uLights[i].enabled.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].enabled", i));
        shader->uLights[i].shadow.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].shadow", i));

        r3d_shader_set_sampler2D_slot(screen.lighting, uLights[i].shadowMap, shadowMapSlot++);
        r3d_shader_set_samplerCube_slot(screen.lighting, uLights[i].shadowCubemap, shadowMapSlot++);
    }

    r3d_shader_disable();
}

void r3d_shader_load_screen_bloom(void)
{
    R3D.shader.screen.bloom.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_BLOOM
    );

    r3d_shader_get_location(screen.bloom, uTexColor);
    r3d_shader_get_location(screen.bloom, uTexBloomBlur);
    r3d_shader_get_location(screen.bloom, uBloomMode);
    r3d_shader_get_location(screen.bloom, uBloomIntensity);

    r3d_shader_enable(screen.bloom);
    r3d_shader_set_sampler2D_slot(screen.bloom, uTexColor, 0);
    r3d_shader_set_sampler2D_slot(screen.bloom, uTexBloomBlur, 1);
    r3d_shader_disable();
}

void r3d_shader_load_screen_fog(void)
{
    R3D.shader.screen.fog.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_FOG
    );

    r3d_shader_get_location(screen.fog, uTexColor);
    r3d_shader_get_location(screen.fog, uTexDepth);
    r3d_shader_get_location(screen.fog, uNear);
    r3d_shader_get_location(screen.fog, uFar);
    r3d_shader_get_location(screen.fog, uFogMode);
    r3d_shader_get_location(screen.fog, uFogColor);
    r3d_shader_get_location(screen.fog, uFogStart);
    r3d_shader_get_location(screen.fog, uFogEnd);
    r3d_shader_get_location(screen.fog, uFogDensity);

    r3d_shader_enable(screen.fog);
    r3d_shader_set_sampler2D_slot(screen.fog, uTexColor, 0);
    r3d_shader_set_sampler2D_slot(screen.fog, uTexDepth, 1);
    r3d_shader_disable();
}

void r3d_shader_load_screen_tonemap(void)
{
    R3D.shader.screen.tonemap.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_TONEMAP
    );

    r3d_shader_get_location(screen.tonemap, uTexColor);
    r3d_shader_get_location(screen.tonemap, uTonemapMode);
    r3d_shader_get_location(screen.tonemap, uTonemapExposure);
    r3d_shader_get_location(screen.tonemap, uTonemapWhite);

    r3d_shader_enable(screen.tonemap);
    r3d_shader_set_sampler2D_slot(screen.tonemap, uTexColor, 0);
    r3d_shader_disable();
}

void r3d_shader_load_screen_adjustment(void)
{
    R3D.shader.screen.adjustment.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_ADJUSTMENT
    );

    r3d_shader_get_location(screen.adjustment, uTexColor);
    r3d_shader_get_location(screen.adjustment, uBrightness);
    r3d_shader_get_location(screen.adjustment, uContrast);
    r3d_shader_get_location(screen.adjustment, uSaturation);

    r3d_shader_enable(screen.adjustment);
    r3d_shader_set_sampler2D_slot(screen.adjustment, uTexColor, 0);
    r3d_shader_disable();
}

void r3d_shader_load_screen_fxaa(void)
{
    R3D.shader.screen.fxaa.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_FXAA
    );

    r3d_shader_get_location(screen.fxaa, uTexture);
    r3d_shader_get_location(screen.fxaa, uTexelSize);

    r3d_shader_enable(screen.fxaa);
    r3d_shader_set_sampler2D_slot(screen.fxaa, uTexture, 0);
    r3d_shader_disable();
}


/* === Texture loading functions === */

void r3d_texture_load_white(void)
{
    static const char DATA = 0xFF;
    R3D.texture.white = rlLoadTexture(&DATA, 1, 1, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1);
}

void r3d_texture_load_black(void)
{
    static const char DATA = 0x00;
    R3D.texture.black = rlLoadTexture(&DATA, 1, 1, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1);
}

void r3d_texture_load_normal(void)
{
    static const Vector3 DATA = { 0.5f, 0.5f, 1.0f };
    R3D.texture.normal = rlLoadTexture(&DATA, 1, 1, PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
}

void r3d_texture_load_ssao_noise(void)
{
#   define R3D_SSAO_NOISE_RESOLUTION 16

    r3d_half_t noise[3 * R3D_SSAO_NOISE_RESOLUTION * R3D_SSAO_NOISE_RESOLUTION] = { 0 };

    for (int i = 0; i < R3D_SSAO_NOISE_RESOLUTION * R3D_SSAO_NOISE_RESOLUTION; i++) {
        noise[i * 3 + 0] = r3d_cvt_fh(((float)GetRandomValue(0, INT16_MAX) / INT16_MAX) * 2.0f - 1.0f);
        noise[i * 3 + 1] = r3d_cvt_fh(((float)GetRandomValue(0, INT16_MAX) / INT16_MAX) * 2.0f - 1.0f);
        noise[i * 3 + 2] = r3d_cvt_fh((float)GetRandomValue(0, INT16_MAX) / INT16_MAX);
    }

    R3D.texture.ssaoNoise = rlLoadTexture(noise,
        R3D_SSAO_NOISE_RESOLUTION,
        R3D_SSAO_NOISE_RESOLUTION,
        PIXELFORMAT_UNCOMPRESSED_R16G16B16,
        1
    );
}

void r3d_texture_load_ssao_kernel(void)
{
#   define R3D_SSAO_KERNEL_SIZE 32

    r3d_half_t kernel[3 * R3D_SSAO_KERNEL_SIZE] = { 0 };

    for (int i = 0; i < R3D_SSAO_KERNEL_SIZE; i++)
    {
        Vector3 sample = { 0 };

        sample.x = ((float)GetRandomValue(0, INT16_MAX) / INT16_MAX) * 2.0f - 1.0f;
        sample.y = ((float)GetRandomValue(0, INT16_MAX) / INT16_MAX) * 2.0f - 1.0f;
        sample.z = (float)GetRandomValue(0, INT16_MAX) / INT16_MAX;

        sample = Vector3Normalize(sample);
        sample = Vector3Scale(sample, (float)GetRandomValue(0, INT16_MAX) / INT16_MAX);

        float scale = (float)i / R3D_SSAO_KERNEL_SIZE;
        scale = Lerp(0.1f, 1.0f, scale * scale);
        sample = Vector3Scale(sample, scale);

        kernel[i * 3 + 0] = r3d_cvt_fh(sample.x);
        kernel[i * 3 + 1] = r3d_cvt_fh(sample.y);
        kernel[i * 3 + 2] = r3d_cvt_fh(sample.z);
    }

    glGenTextures(1, &R3D.texture.ssaoKernel);
    glBindTexture(GL_TEXTURE_1D, R3D.texture.ssaoKernel);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, R3D_SSAO_KERNEL_SIZE, 0, GL_RGB, GL_HALF_FLOAT, kernel);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

void r3d_texture_load_ibl_brdf_lut(void)
{
    Image img = { 0 };

    int special_format_size = 0; // should be 4 or 8 (RG16F or RG32F)
    img.data = r3d_load_dds_from_memory_ext(TEX_IBL_BRDF_LUT, TEX_IBL_BRDF_LUT_SIZE, &img.width, &img.height, &special_format_size);

    if (img.data && (special_format_size == 4 || special_format_size == 8)) {
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        GLenum internal_format = (special_format_size == 4) ? GL_RG16F : GL_RG32F;
        GLenum data_type = (special_format_size == 4) ? GL_HALF_FLOAT : GL_FLOAT;

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img.width, img.height, 0, GL_RG, data_type, img.data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        R3D.texture.iblBrdfLut = texId;
        RL_FREE(img.data);
    }
    else {
        img = LoadImageFromMemory(".dds", TEX_IBL_BRDF_LUT, TEX_IBL_BRDF_LUT_SIZE);
        R3D.texture.iblBrdfLut = rlLoadTexture(img.data, img.width, img.height, img.format, img.mipmaps);
        UnloadImage(img);
    }
}
