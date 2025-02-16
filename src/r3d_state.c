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

#include <rlgl.h>
#include <glad.h>

#include "./embedded/r3d_shaders.h"
#include "./embedded/r3d_textures.h"
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
    gBuffer->matId = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, 1);

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
    rlActiveDrawBuffers(5);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(gBuffer->id, gBuffer->albedo, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->emission, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->normal, RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->orm, RL_ATTACHMENT_COLOR_CHANNEL3, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->matId, RL_ATTACHMENT_COLOR_CHANNEL4, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer->id, gBuffer->depth, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(gBuffer->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }
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

void r3d_framebuffer_load_pingpong(int width, int height)
{
    struct r3d_fb_pingpong_t* pingPong = &R3D.framebuffer.pingPong;

    width /= 2, height /= 2;

    pingPong->id = rlLoadFramebuffer();
    if (pingPong->id == 0) {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
    }

    rlEnableFramebuffer(pingPong->id);

    // Generate (color) buffers
    for (int i = 0; i < 2; i++) {
        pingPong->textures[i] = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, 1);
        glBindTexture(GL_TEXTURE_2D, pingPong->textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Activate the draw buffers for all the attachments
    rlActiveDrawBuffers(1);

    // Attach the textures to the framebuffer
    rlFramebufferAttach(pingPong->id, pingPong->textures[0], RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    // Check if the framebuffer is complete
    if (!rlFramebufferComplete(pingPong->id)) {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
    }

    // Internal data setup
    pingPong->targetTextureIdx = 0;
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
}

void r3d_framebuffer_unload_gbuffer(void)
{
    struct r3d_fb_gbuffer_t* gBuffer = &R3D.framebuffer.gBuffer;

    rlUnloadTexture(gBuffer->albedo);
    rlUnloadTexture(gBuffer->emission);
    rlUnloadTexture(gBuffer->normal);
    rlUnloadTexture(gBuffer->orm);
    rlUnloadTexture(gBuffer->matId);
    rlUnloadTexture(gBuffer->depth);

    rlUnloadFramebuffer(gBuffer->id);

    memset(gBuffer, 0, sizeof(struct r3d_fb_gbuffer_t));
}

void r3d_framebuffer_unload_lit(void)
{
    struct r3d_fb_lit_t* lit = &R3D.framebuffer.lit;

    rlUnloadTexture(lit->color);
    rlUnloadTexture(lit->bright);

    rlUnloadFramebuffer(lit->id);

    memset(lit, 0, sizeof(struct r3d_fb_gbuffer_t));
}

void r3d_framebuffer_unload_pingpong(void)
{
    struct r3d_fb_pingpong_t* pingPong = &R3D.framebuffer.pingPong;

    for (int i = 0; i < sizeof(pingPong->textures) / sizeof(*pingPong->textures); i++) {
        rlUnloadTexture(pingPong->textures[i]);
    }

    rlUnloadFramebuffer(pingPong->id);

    memset(pingPong, 0, sizeof(struct r3d_fb_gbuffer_t));
}

void r3d_framebuffer_unload_post(void)
{
    struct r3d_fb_post_t* post = &R3D.framebuffer.post;

    for (int i = 0; i < sizeof(post->textures) / sizeof(*post->textures); i++) {
        rlUnloadTexture(post->textures[i]);
    }

    rlUnloadFramebuffer(post->id);

    memset(post, 0, sizeof(struct r3d_fb_gbuffer_t));
}


/* === Shader loading functions === */

void r3d_shader_load_generate_gaussian_blur_dual_pass(void)
{
    R3D.shader.generate.gaussianBlurDualPass.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_GENERATE_GAUSSIAN_BLUR_DUAL_PASS
    );

    r3d_shader_get_location(generate.gaussianBlurDualPass, uTexture);
    r3d_shader_get_location(generate.gaussianBlurDualPass, uDirection);

    r3d_shader_enable(generate.gaussianBlurDualPass);
    r3d_shader_set_int(generate.gaussianBlurDualPass, uTexture, 0);
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
    r3d_shader_set_int(generate.cubemapFromEquirectangular, uTexEquirectangular, 0);
    r3d_shader_disable();
}

void r3d_shader_load_generate_irradiance_convolution(void)
{
    R3D.shader.generate.irradianceConvolution.id = rlLoadShaderCode(
        VS_COMMON_CUBEMAP, FS_GENERATE_IRRADIANCE_CONVOLUTION
    );

    r3d_shader_get_location(generate.irradianceConvolution, uMatProj);
    r3d_shader_get_location(generate.irradianceConvolution, uMatView);
    r3d_shader_get_location(generate.irradianceConvolution, uTexCubemap);

    r3d_shader_enable(generate.irradianceConvolution);
    r3d_shader_set_int(generate.irradianceConvolution, uTexCubemap, 0);
    r3d_shader_disable();
}

void r3d_shader_load_generate_prefilter(void)
{
    R3D.shader.generate.prefilter.id = rlLoadShaderCode(
        VS_COMMON_CUBEMAP, FS_GENERATE_PREFILTER
    );

    r3d_shader_get_location(generate.prefilter, uMatProj);
    r3d_shader_get_location(generate.prefilter, uMatView);
    r3d_shader_get_location(generate.prefilter, uTexCubemap);
    r3d_shader_get_location(generate.prefilter, uRoughness);

    r3d_shader_enable(generate.prefilter);
    r3d_shader_set_int(generate.prefilter, uTexCubemap, 0);
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
    r3d_shader_set_int(raster.geometry, uTexAlbedo, 0);
    r3d_shader_set_int(raster.geometry, uTexNormal, 1);
    r3d_shader_set_int(raster.geometry, uTexEmission, 2);
    r3d_shader_set_int(raster.geometry, uTexOcclusion, 3);
    r3d_shader_set_int(raster.geometry, uTexRoughness, 4);
    r3d_shader_set_int(raster.geometry, uTexMetalness, 5);
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
    r3d_shader_get_location(raster.skybox, uTexSkybox);

    r3d_shader_enable(raster.skybox);
    r3d_shader_set_int(raster.skybox, uTexSkybox, 0);
    r3d_shader_disable();
}

void r3d_shader_load_screen_lighting(void)
{
    R3D.shader.screen.lighting.id = rlLoadShaderCode(VS_COMMON_SCREEN, FS_SCREEN_LIGHTING);
    r3d_shader_screen_lighting_t* shader = &R3D.shader.screen.lighting;

    for (int i = 0; i < R3D_SHADER_NUM_LIGHTS; i++) {
        shader->uLights[i].color.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].color", i));
        shader->uLights[i].position.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].position", i));
        shader->uLights[i].direction.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].direction", i));
        shader->uLights[i].energy.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].energy", i));
        shader->uLights[i].range.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].range", i));
        shader->uLights[i].attenuation.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].attenuation", i));
        shader->uLights[i].innerCutOff.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].innerCutOff", i));
        shader->uLights[i].outerCutOff.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].outerCutOff", i));
        shader->uLights[i].type.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].type", i));
        shader->uLights[i].enabled.loc = rlGetLocationUniform(shader->id, TextFormat("uLights[%i].enabled", i));
    }

    r3d_shader_get_location(screen.lighting, uTexAlbedo);
    r3d_shader_get_location(screen.lighting, uTexEmission);
    r3d_shader_get_location(screen.lighting, uTexNormal);
    r3d_shader_get_location(screen.lighting, uTexDepth);
    r3d_shader_get_location(screen.lighting, uTexORM);
    r3d_shader_get_location(screen.lighting, uTexID);
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
    r3d_shader_set_int(screen.lighting, uTexAlbedo, 0);
    r3d_shader_set_int(screen.lighting, uTexEmission, 1);
    r3d_shader_set_int(screen.lighting, uTexNormal, 2);
    r3d_shader_set_int(screen.lighting, uTexDepth, 3);
    r3d_shader_set_int(screen.lighting, uTexORM, 4);
    r3d_shader_set_int(screen.lighting, uTexID, 5);
    r3d_shader_set_int(screen.lighting, uCubeIrradiance, 6);
    r3d_shader_set_int(screen.lighting, uCubePrefilter, 7);
    r3d_shader_set_int(screen.lighting, uTexBrdfLut, 8);
    r3d_shader_disable();

    return shader;
}

void r3d_shader_load_screen_bloom(void)
{
    R3D.shader.screen.bloom.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_BLOOM
    );

    r3d_shader_get_location(screen.bloom, uTexSceneHDR);
    r3d_shader_get_location(screen.bloom, uTexBloomBlurHDR);
    r3d_shader_get_location(screen.bloom, uBloomMode);
    r3d_shader_get_location(screen.bloom, uBloomIntensity);

    r3d_shader_enable(screen.bloom);
    r3d_shader_set_int(screen.bloom, uTexSceneHDR, 0);
    r3d_shader_set_int(screen.bloom, uTexBloomBlurHDR, 1);
    r3d_shader_disable();
}

void r3d_shader_load_screen_fog(void)
{
    R3D.shader.screen.fog.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_FOG
    );

    r3d_shader_get_location(screen.fog, uTexSceneHDR);
    r3d_shader_get_location(screen.fog, uTexSceneDepth);
    r3d_shader_get_location(screen.fog, uNear);
    r3d_shader_get_location(screen.fog, uFar);
    r3d_shader_get_location(screen.fog, uFogMode);
    r3d_shader_get_location(screen.fog, uFogColor);
    r3d_shader_get_location(screen.fog, uFogStart);
    r3d_shader_get_location(screen.fog, uFogEnd);
    r3d_shader_get_location(screen.fog, uFogDensity);

    r3d_shader_enable(screen.fog);
    r3d_shader_set_int(screen.fog, uTexSceneHDR, 0);
    r3d_shader_set_int(screen.fog, uTexSceneDepth, 1);
    r3d_shader_disable();
}

void r3d_shader_load_screen_tonemap(void)
{
    R3D.shader.screen.tonemap.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_TONEMAP
    );

    r3d_shader_get_location(screen.tonemap, uTexSceneHDR);
    r3d_shader_get_location(screen.tonemap, uTonemapMode);
    r3d_shader_get_location(screen.tonemap, uTonemapExposure);
    r3d_shader_get_location(screen.tonemap, uTonemapWhite);

    r3d_shader_enable(screen.tonemap);
    r3d_shader_set_int(screen.tonemap, uTexSceneHDR, 0);
    r3d_shader_disable();
}

void r3d_shader_load_screen_adjustment(void)
{
    R3D.shader.screen.adjustment.id = rlLoadShaderCode(
        VS_COMMON_SCREEN, FS_SCREEN_ADJUSTMENT
    );

    r3d_shader_get_location(screen.adjustment, uTexSceneHDR);
    r3d_shader_get_location(screen.adjustment, uBrightness);
    r3d_shader_get_location(screen.adjustment, uContrast);
    r3d_shader_get_location(screen.adjustment, uSaturation);

    r3d_shader_enable(screen.adjustment);
    r3d_shader_set_int(screen.adjustment, uTexSceneHDR, 0);
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
