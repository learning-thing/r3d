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

#ifndef R3D_DETAILS_DRAWCALL_H
#define R3D_DETAILS_DRAWCALL_H

#include <raylib.h>

/* === Types === */

typedef struct {
    Mesh mesh;
    Matrix transform;
    Material material;
} r3d_drawcall_t;

/* === Functions === */

void r3d_drawcall_raster_geometry(const r3d_drawcall_t* call);
void r3d_drawcall_sort_front_to_back(r3d_drawcall_t* calls, size_t count);

#endif // R3D_DETAILS_DRAWCALL_H
