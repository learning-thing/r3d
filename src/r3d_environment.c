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

#include "./r3d_state.h"
#include <raymath.h>

void R3D_SetBackgroundColor(Color color)
{
	R3D.env.backgroundColor.x = (float)color.r / 255;
	R3D.env.backgroundColor.y = (float)color.g / 255;
	R3D.env.backgroundColor.z = (float)color.b / 255;
}

void R3D_SetAmbientColor(Color color)
{
	R3D.env.ambientColor.x = (float)color.r / 255;
	R3D.env.ambientColor.y = (float)color.g / 255;
	R3D.env.ambientColor.z = (float)color.b / 255;
}

void R3D_EnableSkybox(R3D_Skybox skybox)
{
	R3D.env.sky = skybox;
	R3D.env.useSky = true;
}

void R3D_DisableSkybox(void)
{
	R3D.env.useSky = false;
}

void R3D_SetSkyboxRotation(float pitch, float yaw, float roll)
{
	R3D.env.quatSky = QuaternionFromEuler(pitch, yaw, roll);
}

void R3D_SetBloom(R3D_Bloom mode)
{
	R3D.env.bloomMode = mode;
}

R3D_Bloom R3D_GetBloom(void)
{
	return R3D.env.bloomMode;
}

void R3D_SetBloomIntensity(float value)
{
	R3D.env.bloomIntensity = value;
}

float R3D_GetBloomIntensity(void)
{
	return R3D.env.bloomIntensity;
}

void R3D_SetBloomHdrThreshold(float value)
{
	R3D.env.bloomHdrThreshold = value;
}

float R3D_GetBloomHdrThreshold(void)
{
	return R3D.env.bloomHdrThreshold;
}

void R3D_SetFogMode(R3D_Fog mode)
{
	R3D.env.fogMode = mode;
}

R3D_Fog R3D_GetFogMode(void)
{
	return R3D.env.fogMode;
}

void R3D_SetFogColor(Color color)
{
	R3D.env.fogColor.x = (float)color.r / 255;
	R3D.env.fogColor.y = (float)color.g / 255;
	R3D.env.fogColor.z = (float)color.b / 255;
}

Color R3D_GetFogColor(void)
{
	Color color = { 0 };
	color.r = R3D.env.fogColor.x * 255;
	color.g = R3D.env.fogColor.y * 255;
	color.b = R3D.env.fogColor.z * 255;
	color.a = 255;
	return color;
}

void R3D_SetFogStart(float value)
{
	R3D.env.fogStart = value;
}

float R3D_GetFogStart(void)
{
	return R3D.env.fogStart;
}

void R3D_SetFogEnd(float value)
{
	R3D.env.fogEnd = value;
}

float R3D_GetFogEnd(void)
{
	return R3D.env.fogEnd;
}

void R3D_SetFogDensity(float value)
{
	R3D.env.fogDensity = value;
}

float R3D_GetFogDensity(void)
{
	return R3D.env.fogDensity;
}

void R3D_SetTonemapMode(R3D_Tonemap mode)
{
	R3D.env.tonemapMode = mode;
}

R3D_Tonemap R3D_GetTonemapMode(void)
{
	return R3D.env.tonemapMode;
}

void R3D_SetTonemapExposure(float value)
{
	R3D.env.tonemapExposure = value;
}

float R3D_GetTonemapExposure(void)
{
	return R3D.env.tonemapExposure;
}

void R3D_SetTonemapWhite(float value)
{
	R3D.env.tonemapWhite = value;
}

float R3D_GetTonemapWhite(void)
{
	return R3D.env.tonemapExposure;
}

void R3D_SetBrightness(float value)
{
	R3D.env.brightness = value;
}

float R3D_GetBrightness(void)
{
	return R3D.env.brightness;
}

void R3D_SetContrast(float value)
{
	R3D.env.contrast = value;
}

float R3D_GetContrast(void)
{
	return R3D.env.contrast;
}

void R3D_SetSaturation(float value)
{
	R3D.env.saturation = value;
}

float R3D_GetSaturation(void)
{
	return R3D.env.saturation;
}
