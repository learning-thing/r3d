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
#include <raylib.h>

void R3D_DrawBufferAlbedo(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.gbuffer.albedo,
		.width = R3D.framebuffer.gbuffer.width,
		.height = R3D.framebuffer.gbuffer.height
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, 0, WHITE
	);
}

void R3D_DrawBufferNormal(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.gbuffer.normal,
		.width = R3D.framebuffer.gbuffer.width,
		.height = R3D.framebuffer.gbuffer.height
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, 0, WHITE
	);
}

void R3D_DrawBufferORM(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.gbuffer.orm,
		.width = R3D.framebuffer.gbuffer.width,
		.height = R3D.framebuffer.gbuffer.height
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, 0, WHITE
	);
}
