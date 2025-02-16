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
		.id = R3D.framebuffer.gBuffer.albedo,
		.width = R3D.state.resolutionW,
		.height = R3D.state.resolutionH
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
	);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}

void R3D_DrawBufferEmission(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.gBuffer.emission,
		.width = R3D.state.resolutionW,
		.height = R3D.state.resolutionH
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
	);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}

void R3D_DrawBufferNormal(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.gBuffer.normal,
		.width = R3D.state.resolutionW,
		.height = R3D.state.resolutionH
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
	);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}

void R3D_DrawBufferORM(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.gBuffer.orm,
		.width = R3D.state.resolutionW,
		.height = R3D.state.resolutionH
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
	);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}

void R3D_DrawBufferSSAO(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.pingPongSSAO.textures[!R3D.framebuffer.pingPongSSAO.targetTextureIdx],
		.width = R3D.state.resolutionW / 2,
		.height = R3D.state.resolutionH / 2
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) {
		x, y, w, h
	}, (Vector2) { 0 }, 0, WHITE
			);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}

void R3D_DrawBufferBrightColors(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.lit.bright,
		.width = R3D.state.resolutionW,
		.height = R3D.state.resolutionH
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) {
		x, y, w, h
	}, (Vector2) { 0 }, 0, WHITE
			);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}

void R3D_DrawBufferBloom(float x, float y, float w, float h)
{
	Texture2D tex = {
		.id = R3D.framebuffer.pingPongBloom.textures[!R3D.framebuffer.pingPongBloom.targetTextureIdx],
		.width = R3D.state.resolutionW / 2,
		.height = R3D.state.resolutionH / 2
	};

	DrawTexturePro(
		tex, (Rectangle) { 0, 0, tex.width, tex.height },
		(Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0, WHITE
	);

	DrawRectangleLines(x, y, w, h, (Color) { 255, 0, 0, 255 });
}
