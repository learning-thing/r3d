/* r3d_kinematics.c -- R3D Kinematics Module.
 *
 * Copyright (c) 2025-2026 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <r3d/r3d_kinematics.h>
#include <raymath.h>
#include <stddef.h>
#include <float.h>

#include "./common/r3d_math.h"

// ========================================
// PUBLIC API
// ========================================

Vector3 R3D_ClipVelocity(Vector3 velocity, Vector3 normal)
{
    float dot = Vector3DotProduct(velocity, normal);
    return Vector3Subtract(velocity, Vector3Scale(normal, dot));
}

Vector3 R3D_ReflectVelocity(Vector3 velocity, Vector3 normal, float bounciness)
{
    float dot = Vector3DotProduct(velocity, normal);
    Vector3 reflection = Vector3Subtract(velocity, Vector3Scale(normal, 2.0f * dot));
    return Vector3Scale(reflection, bounciness);
}

Vector3 R3D_SlideVelocity(Vector3 velocity, R3D_SweepCollision collision, Vector3* outNormal)
{
    if (!collision.hit) {
        if (outNormal) *outNormal = (Vector3){0};
        return velocity;
    }

    if (outNormal) *outNormal = collision.normal;

    float safeTime = fmaxf(0.0f, collision.time - 0.001f);
    Vector3 safeVelocity = Vector3Scale(velocity, safeTime);
    Vector3 remainingVelocity = Vector3Scale(velocity, 1.0f - safeTime);
    Vector3 slidedRemaining = R3D_ClipVelocity(remainingVelocity, collision.normal);

    return Vector3Add(safeVelocity, slidedRemaining);
}

Vector3 R3D_SlideSphereBoundingBox(Vector3 center, float radius, Vector3 velocity, BoundingBox box, Vector3* outNormal)
{
    return R3D_SlideVelocity(velocity, R3D_SweepSphereBoundingBox(center, radius, velocity, box), outNormal);
}

Vector3 R3D_SlideSphereMesh(Vector3 center, float radius, Vector3 velocity, R3D_MeshData mesh, Matrix transform, Vector3* outNormal)
{
    return R3D_SlideVelocity(velocity, R3D_SweepSphereMesh(center, radius, velocity, mesh, transform), outNormal);
}

Vector3 R3D_SlideCapsuleBoundingBox(R3D_Capsule capsule, Vector3 velocity, BoundingBox box, Vector3* outNormal)
{
    return R3D_SlideVelocity(velocity, R3D_SweepCapsuleBoundingBox(capsule, velocity, box), outNormal);
}

Vector3 R3D_SlideCapsuleMesh(R3D_Capsule capsule, Vector3 velocity, R3D_MeshData mesh, Matrix transform, Vector3* outNormal)
{
    return R3D_SlideVelocity(velocity, R3D_SweepCapsuleMesh(capsule, velocity, mesh, transform), outNormal);
}

bool R3D_DepenetrateSphereBoundingBox(Vector3* center, float radius, BoundingBox box, float* outPenetration)
{
    Vector3 closestPoint = R3D_ClosestPointOnBox(*center, box);
    Vector3 delta = Vector3Subtract(*center, closestPoint);
    float distSq = Vector3LengthSqr(delta);
    float radiusSq = radius * radius;

    if (distSq >= radiusSq) return false;

    float dist = sqrtf(distSq);
    float penetration = radius - dist;

    Vector3 direction = dist > 1e-6f ? Vector3Scale(delta, 1.0f / dist) : (Vector3){0, 1, 0};
    *center = Vector3Add(*center, Vector3Scale(direction, penetration));

    if (outPenetration) *outPenetration = penetration;
    return true;
}

bool R3D_DepenetrateCapsuleBoundingBox(R3D_Capsule* capsule, BoundingBox box, float* outPenetration)
{
    Vector3 closestOnSegment = R3D_ClosestPointOnSegment(
        R3D_ClosestPointOnBox(capsule->start, box),
        capsule->start,
        capsule->end
    );

    Vector3 closestOnBox = R3D_ClosestPointOnBox(closestOnSegment, box);
    Vector3 delta = Vector3Subtract(closestOnSegment, closestOnBox);
    float distSq = Vector3LengthSqr(delta);
    float radiusSq = capsule->radius * capsule->radius;

    if (distSq >= radiusSq) return false;

    float dist = sqrtf(distSq);
    float penetration = capsule->radius - dist;

    Vector3 direction = dist > 1e-6f ? Vector3Scale(delta, 1.0f / dist) : (Vector3){0, 1, 0};
    Vector3 correction = Vector3Scale(direction, penetration);

    capsule->start = Vector3Add(capsule->start, correction);
    capsule->end = Vector3Add(capsule->end, correction);

    if (outPenetration) *outPenetration = penetration;
    return true;
}

bool R3D_CheckSphereSupportBoundingBox(Vector3 center, float radius, Vector3 direction, float distance, R3D_BoundingBox box, RayCollision* outHit)
{
    RayCollision hit = R3D_RaycastBoundingBox((Ray) {center, direction}, box);
    bool supported = hit.hit && hit.distance <= (radius + distance);
    if (outHit) *outHit = hit;
    return supported;
}

bool R3D_CheckSphereSupportMesh(Vector3 center, float radius, Vector3 direction, float distance, R3D_MeshData mesh, Matrix transform, RayCollision* outHit)
{
    RayCollision hit = R3D_RaycastMesh((Ray) {center, direction}, mesh, transform);
    bool supported = hit.hit && hit.distance <= (radius + distance);
    if (outHit) *outHit = hit;
    return supported;
}

bool R3D_CheckCapsuleSupportBoundingBox(R3D_Capsule capsule, Vector3 direction, float distance, R3D_BoundingBox box, RayCollision* outHit)
{
    RayCollision hit = R3D_RaycastBoundingBox((Ray) {capsule.start, direction}, box);
    bool supported = hit.hit && hit.distance <= (capsule.radius + distance);
    if (outHit) *outHit = hit;
    return supported;
}

bool R3D_CheckCapsuleSupportMesh(R3D_Capsule capsule, Vector3 direction, float distance, R3D_MeshData mesh, Matrix transform, RayCollision* outHit)
{
    RayCollision hit = R3D_RaycastMesh((Ray) {capsule.start, direction}, mesh, transform);
    bool supported = hit.hit && hit.distance <= (capsule.radius + distance);
    if (outHit) *outHit = hit;
    return supported;
}

R3D_SweepCollision R3D_SweepSpherePoint(Vector3 center, float radius, Vector3 velocity, Vector3 point)
{
    R3D_SweepCollision result = {0};
    
    Vector3 m = Vector3Subtract(center, point);
    float c = Vector3DotProduct(m, m) - radius * radius;

    if (c <= 0.0f) {
        result.hit = true;
        result.time = 0.0f;
        result.point = Vector3Add(point, Vector3Scale(Vector3Normalize(m), radius));
        result.normal = Vector3Normalize(m);
        return result;
    }

    float a = Vector3DotProduct(velocity, velocity);
    float b = Vector3DotProduct(m, velocity);
    float discr = b * b - a * c;
    
    if (discr < 0.0f) return result;

    float t = (-b - sqrtf(discr)) / a;
    if (t < 0.0f || t > 1.0f) return result;

    Vector3 hit = Vector3Add(center, Vector3Scale(velocity, t));
    
    result.hit = true;
    result.time = t;
    result.point = hit;
    result.normal = Vector3Normalize(Vector3Subtract(hit, point));
    return result;
}

R3D_SweepCollision R3D_SweepSphereSegment(Vector3 center, float radius, Vector3 velocity, Vector3 a, Vector3 b)
{
    R3D_SweepCollision result = {0};
    
    Vector3 d = Vector3Subtract(b, a);
    Vector3 m = Vector3Subtract(center, a);

    float dd = Vector3DotProduct(d, d);
    float md = Vector3DotProduct(m, d);
    float nd = Vector3DotProduct(velocity, d);

    float a0 = dd * Vector3DotProduct(velocity, velocity) - nd * nd;
    float b0 = dd * Vector3DotProduct(m, velocity) - md * nd;
    float c0 = dd * (Vector3DotProduct(m, m) - radius * radius) - md * md;

    if (fabsf(a0) < 1e-8f) return result;

    float discr = b0 * b0 - a0 * c0;
    if (discr < 0.0f) return result;

    float t = (-b0 - sqrtf(discr)) / a0;
    if (t < 0.0f || t > 1.0f) return result;

    float s = (md + t * nd) / dd;
    if (s < 0.0f || s > 1.0f) return result;

    Vector3 hit = Vector3Add(center, Vector3Scale(velocity, t));
    Vector3 closest = Vector3Add(a, Vector3Scale(d, s));

    result.hit = true;
    result.time = t;
    result.point = hit;
    result.normal = Vector3Normalize(Vector3Subtract(hit, closest));

    return result;
}

R3D_SweepCollision R3D_SweepSphereTrianglePlane(Vector3 center, float radius, Vector3 velocity, Vector3 a, Vector3 b, Vector3 c)
{
    R3D_SweepCollision result = {0};
    
    Vector3 ab = Vector3Subtract(b, a);
    Vector3 ac = Vector3Subtract(c, a);
    Vector3 normal = Vector3Normalize(Vector3CrossProduct(ab, ac));

    float dist = Vector3DotProduct(Vector3Subtract(center, a), normal);
    float denom = Vector3DotProduct(velocity, normal);

    // Moving away or parallel
    if (denom >= 0.0f) return result;

    float t = (radius - dist) / denom;
    if (t < 0.0f || t > 1.0f) return result;

    Vector3 hitPoint = Vector3Add(center, Vector3Scale(velocity, t));
    Vector3 projected = Vector3Subtract(hitPoint, Vector3Scale(normal, radius));

    Vector3 closest = R3D_ClosestPointOnTriangle(projected, a, b, c);
    float d2 = Vector3LengthSqr(Vector3Subtract(projected, closest));

    if (d2 > 1e-6f) return result;

    result.hit = true;
    result.time = t;
    result.point = hitPoint;
    result.normal = normal;

    return result;
}

R3D_SweepCollision R3D_SweepSphereTriangle(Vector3 center, float radius, Vector3 velocity, Vector3 a, Vector3 b, Vector3 c)
{
    R3D_SweepCollision result = {0};
    result.time = 1.0f;

    R3D_SweepCollision faceHit = R3D_SweepSphereTrianglePlane(center, radius, velocity, a, b, c);
    if (faceHit.hit && faceHit.time < result.time) {
        result = faceHit;
    }

    Vector3 edges[3][2] = {{a, b}, {b, c}, {c, a}};
    for (int i = 0; i < 3; i++) {
        R3D_SweepCollision edgeHit = R3D_SweepSphereSegment(center, radius, velocity, edges[i][0], edges[i][1]);
        if (edgeHit.hit && edgeHit.time < result.time) {
            result = edgeHit;
        }
    }

    Vector3 verts[3] = {a, b, c};
    for (int i = 0; i < 3; i++) {
        R3D_SweepCollision vertHit = R3D_SweepSpherePoint(center, radius, velocity, verts[i]);
        if (vertHit.hit && vertHit.time < result.time) {
            result = vertHit;
        }
    }

    return result;
}

R3D_SweepCollision R3D_SweepSphereBoundingBox(Vector3 center, float radius, Vector3 velocity, BoundingBox box)
{
    R3D_SweepCollision collision = {0};

    float velocityLength = Vector3Length(velocity);
    if (velocityLength < 1e-6f) return collision;

    BoundingBox expandedBox = {
        Vector3Subtract(box.min, (Vector3){radius, radius, radius}),
        Vector3Add(box.max, (Vector3){radius, radius, radius})
    };

    Ray ray = {center, Vector3Scale(velocity, 1.0f / velocityLength)};
    RayCollision hit = GetRayCollisionBox(ray, expandedBox);

    if (hit.hit && hit.distance <= velocityLength) {
        collision.hit = true;
        collision.time = hit.distance / velocityLength;
        collision.point = hit.point;
        collision.normal = hit.normal;
    }

    return collision;
}

R3D_SweepCollision R3D_SweepSphereMesh(Vector3 center, float radius, Vector3 velocity, R3D_MeshData mesh, Matrix transform)
{
    R3D_SweepCollision result = {0};
    result.time = 1.0f;

    bool useIndices = (mesh.indices != NULL);
    int triangleCount = useIndices ? (mesh.indexCount / 3) : (mesh.vertexCount / 3);

    for (int i = 0; i < triangleCount; i++)
    {
        Vector3 v0, v1, v2;

        if (useIndices) {
            v0 = mesh.vertices[mesh.indices[i * 3    ]].position;
            v1 = mesh.vertices[mesh.indices[i * 3 + 1]].position;
            v2 = mesh.vertices[mesh.indices[i * 3 + 2]].position;
        }
        else {
            v0 = mesh.vertices[i * 3    ].position;
            v1 = mesh.vertices[i * 3 + 1].position;
            v2 = mesh.vertices[i * 3 + 2].position;
        }

        Vector3 a = r3d_vector3_transform(v0, &transform);
        Vector3 b = r3d_vector3_transform(v1, &transform);
        Vector3 c = r3d_vector3_transform(v2, &transform);

        R3D_SweepCollision hit = R3D_SweepSphereTriangle(center, radius, velocity, a, b, c);
        if (hit.hit && hit.time < result.time) result = hit;
    }

    return result;
}

R3D_SweepCollision R3D_SweepCapsuleBoundingBox(R3D_Capsule capsule, Vector3 velocity, BoundingBox box)
{
    R3D_SweepCollision collision = {0};

    float velocityLength = Vector3Length(velocity);
    if (velocityLength < 1e-6f) return collision;

    BoundingBox expandedBox = {
        Vector3Subtract(box.min, (Vector3){capsule.radius, capsule.radius, capsule.radius}),
        Vector3Add(box.max, (Vector3){capsule.radius, capsule.radius, capsule.radius})
    };

    Vector3 velocityDir = Vector3Scale(velocity, 1.0f / velocityLength);
    Vector3 capsuleAxis = Vector3Subtract(capsule.end, capsule.start);

    RayCollision bestHit = {0};
    bestHit.distance = FLT_MAX;
    bool foundHit = false;

    const int samples = 3;
    for (int i = 0; i < samples; i++)
    {
        float t = (float)i / (float)(samples - 1);
        Vector3 samplePoint = Vector3Add(capsule.start, Vector3Scale(capsuleAxis, t));

        Ray ray = {samplePoint, velocityDir};
        RayCollision hit = GetRayCollisionBox(ray, expandedBox);

        if (hit.hit && hit.distance <= velocityLength && hit.distance < bestHit.distance) {
            bestHit = hit;
            foundHit = true;
        }
    }

    if (foundHit) {
        collision.hit = true;
        collision.time = bestHit.distance / velocityLength;
        collision.point = bestHit.point;
        collision.normal = bestHit.normal;
    }

    return collision;
}

R3D_SweepCollision R3D_SweepCapsuleMesh(R3D_Capsule capsule, Vector3 velocity, R3D_MeshData mesh, Matrix transform)
{
    R3D_SweepCollision result = {0};
    result.time = 1.0f;

    for (int i = 0; i < mesh.indexCount; i += 3)
    {
        Vector3 a = r3d_vector3_transform(mesh.vertices[mesh.indices[i]].position, &transform);
        Vector3 b = r3d_vector3_transform(mesh.vertices[mesh.indices[i+1]].position, &transform);
        Vector3 c = r3d_vector3_transform(mesh.vertices[mesh.indices[i+2]].position, &transform);

        // Face plane test
        R3D_SweepCollision faceHit = R3D_SweepSphereTrianglePlane(capsule.start, capsule.radius, velocity, a, b, c);
        if (faceHit.hit && faceHit.time < result.time) result = faceHit;

        faceHit = R3D_SweepSphereTrianglePlane(capsule.end, capsule.radius, velocity, a, b, c);
        if (faceHit.hit && faceHit.time < result.time) result = faceHit;

        // Segment (cylindre)
        R3D_SweepCollision segHit = R3D_SweepSphereSegment(capsule.start, capsule.radius, velocity, a, b);
        if (segHit.hit && segHit.time < result.time) result = segHit;

        segHit = R3D_SweepSphereSegment(capsule.start, capsule.radius, velocity, b, c);
        if (segHit.hit && segHit.time < result.time) result = segHit;

        segHit = R3D_SweepSphereSegment(capsule.start, capsule.radius, velocity, c, a);
        if (segHit.hit && segHit.time < result.time) result = segHit;

        // Vertices (start)
        R3D_SweepCollision vertHit = R3D_SweepSpherePoint(capsule.start, capsule.radius, velocity, a);
        if (vertHit.hit && vertHit.time < result.time) result = vertHit;

        vertHit = R3D_SweepSpherePoint(capsule.start, capsule.radius, velocity, b);
        if (vertHit.hit && vertHit.time < result.time) result = vertHit;

        vertHit = R3D_SweepSpherePoint(capsule.start, capsule.radius, velocity, c);
        if (vertHit.hit && vertHit.time < result.time) result = vertHit;

        // Vertices (end)
        vertHit = R3D_SweepSpherePoint(capsule.end, capsule.radius, velocity, a);
        if (vertHit.hit && vertHit.time < result.time) result = vertHit;

        vertHit = R3D_SweepSpherePoint(capsule.end, capsule.radius, velocity, b);
        if (vertHit.hit && vertHit.time < result.time) result = vertHit;

        vertHit = R3D_SweepSpherePoint(capsule.end, capsule.radius, velocity, c);
        if (vertHit.hit && vertHit.time < result.time) result = vertHit;
    }

    return result;
}
