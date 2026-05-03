/* r3d_shape.c -- R3D Shape Module.
 *
 * Copyright (c) 2025-2026 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <r3d/r3d_shape.h>
#include <raymath.h>
#include <stddef.h>
#include <float.h>

#include "./common/r3d_math.h"

// ========================================
// INLINE FUNCTIONS
// ========================================

static inline bool raycast_triangle(
    float* outT, Vector3* outEdge1,  Vector3* outEdge2,
    Vector3 localOrigin, Vector3 localDirection,
    Vector3 v0, Vector3 v1, Vector3 v2)
{
    // Möller-Trumbore ray triangle intersection algorithm
    // See: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

    Vector3 edge1 = Vector3Subtract(v1, v0);
    Vector3 edge2 = Vector3Subtract(v2, v0);

    // If negative: it's a backface
    // If zero: ray parallel to triangle
    Vector3 h = Vector3CrossProduct(localDirection, edge2);
    float a = Vector3DotProduct(edge1, h);
    if (a < 1e-5f) return false;

    float f = 1.0f / a;

    Vector3 s = Vector3Subtract(localOrigin, v0);
    float u = f * Vector3DotProduct(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    Vector3 q = Vector3CrossProduct(s, edge1);
    float v = f * Vector3DotProduct(localDirection, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * Vector3DotProduct(edge2, q);
    if (t < 1e-5f) return false;

    *outT = t;
    *outEdge1 = edge1;
    *outEdge2 = edge2;

    return true;
}

static inline void raycast_mesh_vertices(
    float* closestT, Vector3* closestEdge1,  Vector3* closestEdge2,
    const R3D_Vertex* vertices, int triangleCount,
    Vector3 localOrigin, Vector3 localDirection)
{
    for (int i = 0; i < triangleCount; i++)
    {
        int baseIdx = i * 3;
        Vector3 v0 = vertices[baseIdx].position;
        Vector3 v1 = vertices[baseIdx + 1].position;
        Vector3 v2 = vertices[baseIdx + 2].position;

        float t;
        Vector3 edge1, edge2;
        if (raycast_triangle(&t, &edge1, &edge2, localOrigin, localDirection, v0, v1, v2)) {
            if (t < *closestT) {
                *closestT = t;
                *closestEdge1 = edge1;
                *closestEdge2 = edge2;
            }
        }
    }
}

static inline void raycast_mesh_indexed(
    float* closestT, Vector3* closestEdge1,  Vector3* closestEdge2,
    const R3D_Vertex* vertices, const uint32_t* indices, int triangleCount,
    Vector3 localOrigin, Vector3 localDirection)
{
    for (int i = 0; i < triangleCount; i++)
    {
        int baseIdx = i * 3;
        Vector3 v0 = vertices[indices[baseIdx]].position;
        Vector3 v1 = vertices[indices[baseIdx + 1]].position;
        Vector3 v2 = vertices[indices[baseIdx + 2]].position;

        float t;
        Vector3 edge1, edge2;
        if (raycast_triangle(&t, &edge1, &edge2, localOrigin, localDirection, v0, v1, v2)) {
            if (t < *closestT) {
                *closestT = t;
                *closestEdge1 = edge1;
                *closestEdge2 = edge2;
            }
        }
    }
}

// ========================================
// PUBLIC API
// ========================================

R3D_BoundingBox R3D_GetBoundingBox(Vector3 center, Vector3 halfExtents)
{
    R3D_BoundingBox aabb;

    aabb.min = Vector3Subtract(center, halfExtents);
    aabb.max = Vector3Add(center, halfExtents);

    return aabb;
}

R3D_OrientedBox R3D_GetOrientedBox(BoundingBox aabb, Matrix transform)
{
    R3D_OrientedBox obb;

    obb.halfExtents.x = (aabb.max.x - aabb.min.x) * 0.5f;
    obb.halfExtents.y = (aabb.max.y - aabb.min.y) * 0.5f;
    obb.halfExtents.z = (aabb.max.z - aabb.min.z) * 0.5f;

    Vector3 localCenter = {
        (aabb.min.x + aabb.max.x) * 0.5f,
        (aabb.min.y + aabb.max.y) * 0.5f,
        (aabb.min.z + aabb.max.z) * 0.5f
    };

    obb.center.x = transform.m0*localCenter.x + transform.m4*localCenter.y + transform.m8*localCenter.z + transform.m12;
    obb.center.y = transform.m1*localCenter.x + transform.m5*localCenter.y + transform.m9*localCenter.z + transform.m13;
    obb.center.z = transform.m2*localCenter.x + transform.m6*localCenter.y + transform.m10*localCenter.z + transform.m14;

    obb.axisX = (Vector3){transform.m0, transform.m1, transform.m2};
    obb.axisY = (Vector3){transform.m4, transform.m5, transform.m6};
    obb.axisZ = (Vector3){transform.m8, transform.m9, transform.m10};

    return obb;
}

bool R3D_CheckCollisionCapsuleBox(R3D_Capsule capsule, BoundingBox box)
{
    Vector3 closestOnSegment = R3D_ClosestPointOnSegment(
        R3D_ClosestPointOnBox(capsule.start, box),
        capsule.start,
        capsule.end
    );

    Vector3 closestOnBox = R3D_ClosestPointOnBox(closestOnSegment, box);
    float distSq = Vector3DistanceSqr(closestOnBox, closestOnSegment);

    return distSq <= (capsule.radius * capsule.radius);
}

bool R3D_CheckCollisionCapsuleSphere(R3D_Capsule capsule, Vector3 center, float radius)
{
    Vector3 closestPoint = R3D_ClosestPointOnSegment(center, capsule.start, capsule.end);
    float distSq = Vector3DistanceSqr(center, closestPoint);
    float radiusSum = capsule.radius + radius;

    return distSq <= (radiusSum * radiusSum);
}

bool R3D_CheckCollisionCapsules(R3D_Capsule a, R3D_Capsule b)
{
    Vector3 dirA = Vector3Subtract(a.end, a.start);
    Vector3 dirB = Vector3Subtract(b.end, b.start);
    Vector3 r = Vector3Subtract(a.start, b.start);

    float dotAA = Vector3DotProduct(dirA, dirA);
    float dotBB = Vector3DotProduct(dirB, dirB);
    float dotAB = Vector3DotProduct(dirA, dirB);
    float dotAR = Vector3DotProduct(dirA, r);
    float dotBR = Vector3DotProduct(dirB, r);

    float denom = dotAA * dotBB - dotAB * dotAB;
    float s = 0.0f, t = 0.0f;

    if (denom > 1e-6f) {
        s = (dotAB * dotBR - dotBB * dotAR) / denom;
        s = fmaxf(0.0f, fminf(1.0f, s));
        t = (dotAB * s + dotBR) / dotBB;

        if (t < 0.0f) {
            t = 0.0f;
            s = fmaxf(0.0f, fminf(1.0f, -dotAR / dotAA));
        }
        else if (t > 1.0f) {
            t = 1.0f;
            s = fmaxf(0.0f, fminf(1.0f, (dotAB - dotAR) / dotAA));
        }
    }
    else {
        s = 0.5f;
        t = fmaxf(0.0f, fminf(1.0f, dotBR / dotBB));
    }

    Vector3 closestA = Vector3Add(a.start, Vector3Scale(dirA, s));
    Vector3 closestB = Vector3Add(b.start, Vector3Scale(dirB, t));

    float distSq = Vector3DistanceSqr(closestA, closestB);
    float radiusSum = a.radius + b.radius;

    return distSq <= (radiusSum * radiusSum);
}

bool R3D_CheckCollisionCapsuleMesh(R3D_Capsule capsule, R3D_MeshData mesh, Matrix transform)
{
    Vector3 axis = Vector3Subtract(capsule.end, capsule.start);
    float radiusSq = capsule.radius * capsule.radius;

    bool useIndices = (mesh.indices != NULL);
    int triangleCount = useIndices ? (mesh.indexCount / 3) : (mesh.vertexCount / 3);

    for (int i = 0; i < triangleCount; i++)
    {
        Vector3 v0, v1, v2;

        if (useIndices) {
            v0 = mesh.vertices[mesh.indices[i*3    ]].position;
            v1 = mesh.vertices[mesh.indices[i*3 + 1]].position;
            v2 = mesh.vertices[mesh.indices[i*3 + 2]].position;
        }
        else {
            v0 = mesh.vertices[i*3    ].position;
            v1 = mesh.vertices[i*3 + 1].position;
            v2 = mesh.vertices[i*3 + 2].position;
        }

        Vector3 a = r3d_vector3_transform(mesh.vertices[mesh.indices[i]].position, &transform);
        Vector3 b = r3d_vector3_transform(mesh.vertices[mesh.indices[i+1]].position, &transform);
        Vector3 c = r3d_vector3_transform(mesh.vertices[mesh.indices[i+2]].position, &transform);

        const int samples = 5;
        for (int s = 0; s < samples; s++)
        {
            float t = (float)s / (samples - 1);
            Vector3 p = Vector3Add(capsule.start, Vector3Scale(axis, t));

            Vector3 closest = R3D_ClosestPointOnTriangle(p, a, b, c);
            if (Vector3LengthSqr(Vector3Subtract(closest, p)) <= radiusSq) {
                return true;
            }
        }
    }

    return false;
}

R3D_Penetration R3D_CheckPenetrationCapsuleBox(R3D_Capsule capsule, BoundingBox box)
{
    R3D_Penetration result = {0};

    Vector3 closestOnSegment = R3D_ClosestPointOnSegment(
        R3D_ClosestPointOnBox(capsule.start, box),
        capsule.start,
        capsule.end
    );

    Vector3 closestOnBox = R3D_ClosestPointOnBox(closestOnSegment, box);
    Vector3 delta = Vector3Subtract(closestOnSegment, closestOnBox);

    float distSq = Vector3LengthSqr(delta);
    float radiusSq = capsule.radius * capsule.radius;

    if (distSq >= radiusSq) return result;

    float dist = sqrtf(distSq);
    result.collides = true;
    result.depth = capsule.radius - dist;

    if (dist > 1e-6f) {
        result.normal = Vector3Scale(delta, 1.0f / dist);
    }
    else {
        Vector3 boxCenter = {
            (box.min.x + box.max.x) * 0.5f,
            (box.min.y + box.max.y) * 0.5f,
            (box.min.z + box.max.z) * 0.5f
        };
        Vector3 toCenter = Vector3Subtract(closestOnSegment, boxCenter);

        float ax = fabsf(toCenter.x);
        float ay = fabsf(toCenter.y);
        float az = fabsf(toCenter.z);

        if (ax >= ay && ax >= az) {
            result.normal = (Vector3){toCenter.x > 0 ? 1.0f : -1.0f, 0, 0};
        }
        else if (ay >= az) {
            result.normal = (Vector3){0, toCenter.y > 0 ? 1.0f : -1.0f, 0};
        }
        else {
            result.normal = (Vector3){0, 0, toCenter.z > 0 ? 1.0f : -1.0f};
        }
    }
    
    result.mtv = Vector3Scale(result.normal, result.depth);
    return result;
}

R3D_Penetration R3D_CheckPenetrationCapsuleSphere(R3D_Capsule capsule, Vector3 center, float radius)
{
    R3D_Penetration result = {0};

    Vector3 closestOnSegment = R3D_ClosestPointOnSegment(center, capsule.start, capsule.end);
    Vector3 delta = Vector3Subtract(center, closestOnSegment);

    float distSq = Vector3LengthSqr(delta);
    float combinedRadius = capsule.radius + radius;
    float combinedRadiusSq = combinedRadius * combinedRadius;

    if (distSq >= combinedRadiusSq) return result;

    float dist = sqrtf(distSq);
    result.collides = true;
    result.depth = combinedRadius - dist;

    if (dist > 1e-6f) {
        result.normal = Vector3Scale(delta, 1.0f / dist);
    }
    else {
        Vector3 capsuleDir = Vector3Subtract(capsule.end, capsule.start);
        float capsuleLengthSq = Vector3LengthSqr(capsuleDir);
        
        if (capsuleLengthSq > 1e-6f) {
            result.normal = (Vector3){capsuleDir.y, -capsuleDir.x, 0};
            float normalLengthSq = Vector3LengthSqr(result.normal);

            if (normalLengthSq < 1e-6f) {
                result.normal = (Vector3){0, capsuleDir.z, -capsuleDir.y};
                normalLengthSq = Vector3LengthSqr(result.normal);
            }

            if (normalLengthSq > 1e-6f) {
                result.normal = Vector3Normalize(result.normal);
            }
            else {
                result.normal = (Vector3){0, 1, 0};
            }
        }
        else {
            result.normal = (Vector3){0, 1, 0};
        }
    }

    result.mtv = Vector3Scale(result.normal, result.depth);
    return result;
}

R3D_Penetration R3D_CheckPenetrationCapsules(R3D_Capsule a, R3D_Capsule b)
{
    R3D_Penetration result = {0};

    Vector3 dirA = Vector3Subtract(a.end, a.start);
    Vector3 dirB = Vector3Subtract(b.end, b.start);
    Vector3 r = Vector3Subtract(a.start, b.start);

    float dotAA = Vector3DotProduct(dirA, dirA);
    float dotBB = Vector3DotProduct(dirB, dirB);
    float dotAB = Vector3DotProduct(dirA, dirB);
    float dotAR = Vector3DotProduct(dirA, r);
    float dotBR = Vector3DotProduct(dirB, r);

    float denom = dotAA * dotBB - dotAB * dotAB;

    float s = 0.0f, t = 0.0f;
    
    if (denom > 1e-6f) {
        s = (dotAB * dotBR - dotBB * dotAR) / denom;
        s = fmaxf(0.0f, fminf(1.0f, s));
        t = (dotAB * s + dotBR) / dotBB;

        if (t < 0.0f) {
            t = 0.0f;
            s = fmaxf(0.0f, fminf(1.0f, -dotAR / dotAA));
        }
        else if (t > 1.0f) {
            t = 1.0f;
            s = fmaxf(0.0f, fminf(1.0f, (dotAB - dotAR) / dotAA));
        }
    }
    else {
        s = 0.5f;
        t = fmaxf(0.0f, fminf(1.0f, dotBR / dotBB));
    }

    Vector3 closestA = Vector3Add(a.start, Vector3Scale(dirA, s));
    Vector3 closestB = Vector3Add(b.start, Vector3Scale(dirB, t));

    Vector3 delta = Vector3Subtract(closestA, closestB);
    float distSq = Vector3LengthSqr(delta);
    float combinedRadius = a.radius + b.radius;
    float combinedRadiusSq = combinedRadius * combinedRadius;

    if (distSq >= combinedRadiusSq) return result;

    float dist = sqrtf(distSq);
    result.collides = true;
    result.depth = combinedRadius - dist;

    if (dist > 1e-6f) {
        result.normal = Vector3Scale(delta, 1.0f / dist);
    }
    else {
        Vector3 cross = Vector3CrossProduct(dirA, dirB);
        float crossLengthSq = Vector3LengthSqr(cross);

        if (crossLengthSq > 1e-6f) {
            result.normal = Vector3Normalize(cross);
        }
        else {
            Vector3 perp = (Vector3){dirA.y, -dirA.x, 0};
            float perpLengthSq = Vector3LengthSqr(perp);

            if (perpLengthSq < 1e-6f) {
                perp = (Vector3){0, dirA.z, -dirA.y};
                perpLengthSq = Vector3LengthSqr(perp);
            }

            if (perpLengthSq > 1e-6f) {
                result.normal = Vector3Normalize(perp);
            }
            else {
                result.normal = (Vector3){0, 1, 0};
            }
        }
    }

    result.mtv = Vector3Scale(result.normal, result.depth);
    return result;
}

RayCollision R3D_RaycastMesh(Ray ray, R3D_MeshData mesh, Matrix transform)
{
    RayCollision collision = {0};
    collision.distance = FLT_MAX;

    if (mesh.vertices == NULL) {
        return collision;
    }

    Matrix invTransform = MatrixInvert(transform);
    Vector3 localOrigin = r3d_vector3_transform(ray.position, &invTransform);
    Vector3 localDirection = Vector3Normalize(r3d_vector3_transform_normal(ray.direction, &invTransform));

    int triangleCount = mesh.indices ? (mesh.indexCount / 3) : (mesh.vertexCount / 3);

    Vector3 closestEdge1 = {0};
    Vector3 closestEdge2 = {0};
    float closestT = FLT_MAX;

    if (mesh.indices) {
        raycast_mesh_indexed(
            &closestT, &closestEdge1, &closestEdge2,
            mesh.vertices, mesh.indices, triangleCount,
            localOrigin, localDirection
        );
    }
    else {
        raycast_mesh_vertices(
            &closestT, &closestEdge1, &closestEdge2,
            mesh.vertices, triangleCount,
            localOrigin, localDirection
        );
    }

    if (closestT < FLT_MAX)
    {
        Vector3 closestHitLocal = Vector3Add(localOrigin, Vector3Scale(localDirection, closestT));
        Vector3 normalLocal = Vector3Normalize(Vector3CrossProduct(closestEdge1, closestEdge2));
        Matrix normalMatrix = MatrixTranspose(invTransform);

        collision.hit = true;
        collision.point = r3d_vector3_transform(closestHitLocal, &transform);
        collision.distance = Vector3Distance(ray.position, collision.point);
        collision.normal = Vector3Normalize(r3d_vector3_transform_normal(normalLocal, &normalMatrix));
    }

    return collision;
}

RayCollision R3D_RaycastModel(Ray ray, R3D_Model model, Matrix transform)
{
    RayCollision collision = {0};
    collision.distance = FLT_MAX;

    if (model.meshData == NULL || model.meshCount <= 0) {
        return collision;
    }

    Matrix invTransform = MatrixInvert(transform);
    Vector3 localOrigin = r3d_vector3_transform(ray.position, &invTransform);
    Vector3 localDirection = Vector3Normalize(r3d_vector3_transform_normal(ray.direction, &invTransform));

    Vector3 closestEdge1 = {0};
    Vector3 closestEdge2 = {0};
    float closestT = FLT_MAX;

    // Test each mesh
    for (int meshIdx = 0; meshIdx < model.meshCount; meshIdx++)
    {
        R3D_MeshData mesh = model.meshData[meshIdx];
        if (mesh.vertices == NULL) continue;

        // Per-mesh AABB culling
        RayCollision meshBoxCol = GetRayCollisionBox(ray, model.meshes[meshIdx].aabb);
        if (!meshBoxCol.hit) continue;

        int triangleCount = mesh.indices ? (mesh.indexCount / 3) : (mesh.vertexCount / 3);

        if (mesh.indices) {
            raycast_mesh_indexed(
                &closestT, &closestEdge1, &closestEdge2,
                mesh.vertices, mesh.indices, triangleCount,
                localOrigin, localDirection
            );
        }
        else {
            raycast_mesh_vertices(
                &closestT, &closestEdge1, &closestEdge2,
                mesh.vertices, triangleCount,
                localOrigin, localDirection
            );
        }
    }

    if (closestT < FLT_MAX)
    {
        Vector3 closestHitLocal = Vector3Add(localOrigin, Vector3Scale(localDirection, closestT));
        Vector3 normalLocal = Vector3Normalize(Vector3CrossProduct(closestEdge1, closestEdge2));
        Matrix normalMatrix = MatrixTranspose(invTransform);

        collision.hit = true;
        collision.point = r3d_vector3_transform(closestHitLocal, &transform);
        collision.distance = Vector3Distance(ray.position, collision.point);
        collision.normal = Vector3Normalize(r3d_vector3_transform_normal(normalLocal, &normalMatrix));
    }

    return collision;
}

Vector3 R3D_ClosestPointOnSegment(Vector3 point, Vector3 start, Vector3 end)
{
    Vector3 dir = Vector3Subtract(end, start);
    float lenSq = Vector3LengthSqr(dir);

    if (lenSq < 1e-10f) return start;

    float t = Vector3DotProduct(Vector3Subtract(point, start), dir) / lenSq;
    t = fmaxf(0.0f, fminf(t, 1.0f));

    return Vector3Add(start, Vector3Scale(dir, t));
}

Vector3 R3D_ClosestPointOnTriangle(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
    Vector3 ab = Vector3Subtract(b, a);
    Vector3 ac = Vector3Subtract(c, a);
    Vector3 ap = Vector3Subtract(p, a);

    float d1 = Vector3DotProduct(ab, ap);
    float d2 = Vector3DotProduct(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a;

    Vector3 bp = Vector3Subtract(p, b);
    float d3 = Vector3DotProduct(ab, bp);
    float d4 = Vector3DotProduct(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b;

    Vector3 cp = Vector3Subtract(p, c);
    float d5 = Vector3DotProduct(ab, cp);
    float d6 = Vector3DotProduct(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;

    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return Vector3Add(a, Vector3Scale(ab, v));
    }
        
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float v = d2 / (d2 - d6);
        return Vector3Add(a, Vector3Scale(ac, v));
    }
        
    float va = d3 * d6 - d5 * d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return Vector3Add(b, Vector3Scale(Vector3Subtract(c, b), v));
    }

    float denom = 1.f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;

    return Vector3Add(a, Vector3Add(Vector3Scale(ab, v), Vector3Scale(ac, w)));
}

Vector3 R3D_ClosestPointOnBox(Vector3 point, BoundingBox box)
{
    Vector3 closest;
    closest.x = fmaxf(box.min.x, fminf(point.x, box.max.x));
    closest.y = fmaxf(box.min.y, fminf(point.y, box.max.y));
    closest.z = fmaxf(box.min.z, fminf(point.z, box.max.z));
    return closest;
}
