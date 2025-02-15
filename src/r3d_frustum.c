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

#include <raymath.h>
#include "./r3d_state.h"

/* === Internal functions === */

static inline Vector4 r3d_frustum_normalize_plane(Vector4 plane)
{
    float mag = sqrtf(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    if (mag <= 1e-6f) (Vector4) { 0 };

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

R3D_Frustum R3D_GetFrustum(void)
{
    return R3D_CreateFrustum(
        MatrixMultiply(R3D.state.matView, R3D.state.matProj)
    );
}

R3D_Frustum R3D_CreateFrustum(Matrix matrixViewProjection)
{
    R3D_Frustum frustum = { 0 };

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

bool R3D_IsPointInFrustum(const R3D_Frustum* frustum, Vector3 position)
{
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        if (r3d_frustum_distance_to_plane(frustum->planes[i], position) <= 0) {
            return false;
        }
    }
    return true;
}

bool R3D_IsPointInFrustumXYZ(const R3D_Frustum* frustum, float x, float y, float z)
{
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        if (r3d_frustum_distance_to_plane_xyz(frustum->planes[i], x, y, z) <= 0) {
            return false;
        }
    }
    return true;
}

bool R3D_IsSphereInFrustum(const R3D_Frustum* frustum, Vector3 position, float radius)
{
    for (int i = 0; i < R3D_PLANE_COUNT; i++) {
        if (r3d_frustum_distance_to_plane(frustum->planes[i], position) < -radius) {
            return false;
        }
    }
    return true;
}

bool R3D_IsBoundingBoxInFrustum(const R3D_Frustum* frustum, BoundingBox aabb)
{
    // if any point is in and we are good
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.min.x, aabb.min.y, aabb.min.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.min.x, aabb.max.y, aabb.min.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.max.x, aabb.max.y, aabb.min.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.max.x, aabb.min.y, aabb.min.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.min.x, aabb.min.y, aabb.max.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.min.x, aabb.max.y, aabb.max.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.max.x, aabb.max.y, aabb.max.z)) return true;
    if (R3D_IsPointInFrustumXYZ(frustum, aabb.max.x, aabb.min.y, aabb.max.z)) return true;

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
