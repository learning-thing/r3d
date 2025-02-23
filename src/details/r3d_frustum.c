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

#include "./r3d_frustum.h"

#include <raymath.h>
#include <float.h>

/* === Internal functions === */

static inline Vector4 r3d_frustum_normalize_plane(Vector4 plane)
{
    float mag = sqrtf(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    if (mag <= 1e-6f) return (Vector4) { 0 };

    return Vector4Scale(plane, 1.0f / mag);
}

static inline float r3d_frustum_distance_to_plane(Vector4 plane, Vector3 position)
{
    return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
}

static inline float r3d_frustum_distance_to_plane_xyz(Vector4 plane, float x, float y, float z)
{
    return plane.x * x + plane.y * y + plane.z * z + plane.w;
}

/* === Public functions === */

r3d_frustum_t r3d_frustum_create(Matrix matrixViewProjection)
{
    r3d_frustum_t frustum = { 0 };

    frustum.planes[R3D_PLANE_RIGHT] = r3d_frustum_normalize_plane((Vector4) {
        matrixViewProjection.m3 - matrixViewProjection.m0,
            matrixViewProjection.m7 - matrixViewProjection.m4,
            matrixViewProjection.m11 - matrixViewProjection.m8,
            matrixViewProjection.m15 - matrixViewProjection.m12
    });

    frustum.planes[R3D_PLANE_LEFT] = r3d_frustum_normalize_plane((Vector4) {
        matrixViewProjection.m3 + matrixViewProjection.m0,
            matrixViewProjection.m7 + matrixViewProjection.m4,
            matrixViewProjection.m11 + matrixViewProjection.m8,
            matrixViewProjection.m15 + matrixViewProjection.m12
    });

    frustum.planes[R3D_PLANE_TOP] = r3d_frustum_normalize_plane((Vector4) {
        matrixViewProjection.m3 - matrixViewProjection.m1,
            matrixViewProjection.m7 - matrixViewProjection.m5,
            matrixViewProjection.m11 - matrixViewProjection.m9,
            matrixViewProjection.m15 - matrixViewProjection.m13
    });

    frustum.planes[R3D_PLANE_BOTTOM] = r3d_frustum_normalize_plane((Vector4) {
        matrixViewProjection.m3 + matrixViewProjection.m1,
            matrixViewProjection.m7 + matrixViewProjection.m5,
            matrixViewProjection.m11 + matrixViewProjection.m9,
            matrixViewProjection.m15 + matrixViewProjection.m13
    });

    frustum.planes[R3D_PLANE_BACK] = r3d_frustum_normalize_plane((Vector4) {
        matrixViewProjection.m3 - matrixViewProjection.m2,
            matrixViewProjection.m7 - matrixViewProjection.m6,
            matrixViewProjection.m11 - matrixViewProjection.m10,
            matrixViewProjection.m15 - matrixViewProjection.m14
    });

    frustum.planes[R3D_PLANE_FRONT] = r3d_frustum_normalize_plane((Vector4) {
        matrixViewProjection.m3 + matrixViewProjection.m2,
            matrixViewProjection.m7 + matrixViewProjection.m6,
            matrixViewProjection.m11 + matrixViewProjection.m10,
            matrixViewProjection.m15 + matrixViewProjection.m14
    });

    return frustum;
}

BoundingBox r3d_frustum_get_bounding_box(Matrix matViewProjection)
{
    Matrix matInv = MatrixInvert(matViewProjection);

    // Points in clip space with correct w component
    Vector4 clipCorners[8] = {
        { -1, -1, -1, 1 }, { 1, -1, -1, 1 }, { 1, 1, -1, 1 }, { -1, 1, -1, 1 },  // Near
        { -1, -1, 1, 1 }, { 1, -1, 1, 1 }, { 1, 1, 1, 1 }, { -1, 1, 1, 1 }       // Far
    };

    BoundingBox bbox = {
        .min = (Vector3){ FLT_MAX, FLT_MAX, FLT_MAX },
        .max = (Vector3){ -FLT_MAX, -FLT_MAX, -FLT_MAX }
    };

    for (int i = 0; i < 8; i++) {
        Vector4 p = clipCorners[i];

        // Transform to world space
        float x = p.x * matInv.m0 + p.y * matInv.m4 + p.z * matInv.m8 + p.w * matInv.m12;
        float y = p.x * matInv.m1 + p.y * matInv.m5 + p.z * matInv.m9 + p.w * matInv.m13;
        float z = p.x * matInv.m2 + p.y * matInv.m6 + p.z * matInv.m10 + p.w * matInv.m14;
        float w = p.x * matInv.m3 + p.y * matInv.m7 + p.z * matInv.m11 + p.w * matInv.m15;

        // Perspective divide
        if (fabsf(w) > 1e-6f) {  // Avoid division by very small numbers
            x /= w;
            y /= w;
            z /= w;
        }

        // Update bounding box
        bbox.min.x = fminf(bbox.min.x, x);
        bbox.min.y = fminf(bbox.min.y, y);
        bbox.min.z = fminf(bbox.min.z, z);
        bbox.max.x = fmaxf(bbox.max.x, x);
        bbox.max.y = fmaxf(bbox.max.y, y);
        bbox.max.z = fmaxf(bbox.max.z, z);
    }

    return bbox;
}

bool r3d_frustum_is_point_in(const r3d_frustum_t* frustum, Vector3 position)
{
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        if (r3d_frustum_distance_to_plane(frustum->planes[i], position) <= 0) {
            return false;
        }
    }
    return true;
}

bool r3d_frustum_is_point_in_xyz(const r3d_frustum_t* frustum, float x, float y, float z)
{
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        if (r3d_frustum_distance_to_plane_xyz(frustum->planes[i], x, y, z) <= 0) {
            return false;
        }
    }
    return true;
}

bool r3d_frustum_is_sphere_in(const r3d_frustum_t* frustum, Vector3 position, float radius)
{
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        if (r3d_frustum_distance_to_plane(frustum->planes[i], position) < -radius) {
            return false;
        }
    }
    return true;
}

bool r3d_frustum_is_bounding_box_in(const r3d_frustum_t* frustum, BoundingBox aabb)
{
    // if any point is in and we are good
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.min.x, aabb.min.y, aabb.min.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.min.x, aabb.max.y, aabb.min.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.max.x, aabb.max.y, aabb.min.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.max.x, aabb.min.y, aabb.min.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.min.x, aabb.min.y, aabb.max.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.min.x, aabb.max.y, aabb.max.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.max.x, aabb.max.y, aabb.max.z)) return true;
    if (r3d_frustum_is_point_in_xyz(frustum, aabb.max.x, aabb.min.y, aabb.max.z)) return true;

    // check to see if all points are outside of any one plane, if so the entire box is outside
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        Vector4 plane = frustum->planes[i];
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.min.x, aabb.min.y, aabb.min.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.max.x, aabb.min.y, aabb.min.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.max.x, aabb.max.y, aabb.min.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.min.x, aabb.max.y, aabb.min.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.min.x, aabb.min.y, aabb.max.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.max.x, aabb.min.y, aabb.max.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.max.x, aabb.max.y, aabb.max.z) >= 0) continue;
        if (r3d_frustum_distance_to_plane_xyz(plane, aabb.min.x, aabb.max.y, aabb.max.z) >= 0) continue;
        return false;
    }

    // the box extends outside the frustum but crosses it
    return true;
}
