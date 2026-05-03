/* r3d_kinematics.h -- R3D Kinematics Module.
 *
 * Copyright (c) 2025-2026 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef R3D_KINEMATICS_H
#define R3D_KINEMATICS_H

#include "./r3d_mesh_data.h"
#include "./r3d_shape.h"

/**
 * @defgroup Kinematics
 * @brief Shape state modifiers (movement resolution, depenetration, sliding, sweeping).
 *
 * Read-only queries on shapes belong to the Shape module.
 * @{
 */

// ========================================
// STRUCTS TYPES
// ========================================

/**
 * @brief Collision information from a sweep test
 */
typedef struct R3D_SweepCollision {
    bool hit;           ///< Whether a collision occurred
    float time;         ///< Time of impact [0-1], fraction along velocity vector
    Vector3 point;      ///< World space collision point
    Vector3 normal;     ///< Surface normal at collision point
} R3D_SweepCollision;

// ========================================
// PUBLIC API
// ========================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate slide velocity along surface
 * @param velocity Original velocity
 * @param normal Surface normal (must be normalized)
 * @return Velocity sliding along surface (perpendicular component removed)
 */
R3DAPI Vector3 R3D_SlideVelocity(Vector3 velocity, Vector3 normal);

/**
 * @brief Calculate bounce velocity after collision
 * @param velocity Incoming velocity
 * @param normal Surface normal (must be normalized)
 * @param bounciness Coefficient of restitution (0=no bounce, 1=perfect bounce)
 * @return Reflected velocity
 */
R3DAPI Vector3 R3D_BounceVelocity(Vector3 velocity, Vector3 normal, float bounciness);

/**
 * @brief Slide sphere along bounding box surface, resolving collisions
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Desired movement vector
 * @param box Obstacle bounding box
 * @param outNormal Optional: receives collision normal if collision occurred
 * @return Actual movement applied (may be reduced/redirected by collision)
 */
R3DAPI Vector3 R3D_SlideSphereBoundingBox(Vector3 center, float radius, Vector3 velocity, R3D_BoundingBox box, Vector3* outNormal);

/**
 * @brief Slide sphere along mesh surface, resolving collisions
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Desired movement vector
 * @param mesh Mesh data to collide against
 * @param transform Mesh world transform
 * @param outNormal Optional: receives collision normal if collision occurred
 * @return Actual movement applied (may be reduced/redirected by collision)
 */
R3DAPI Vector3 R3D_SlideSphereMesh(Vector3 center, float radius, Vector3 velocity, R3D_MeshData mesh, Matrix transform, Vector3* outNormal);

/**
 * @brief Slide capsule along bounding box surface, resolving collisions
 * @param capsule Capsule shape
 * @param velocity Desired movement vector
 * @param box Obstacle bounding box
 * @param outNormal Optional: receives collision normal if collision occurred
 * @return Actual movement applied (may be reduced/redirected by collision)
 */
R3DAPI Vector3 R3D_SlideCapsuleBoundingBox(R3D_Capsule capsule, Vector3 velocity, R3D_BoundingBox box, Vector3* outNormal);

/**
 * @brief Slide capsule along mesh surface, resolving collisions
 * @param capsule Capsule shape
 * @param velocity Desired movement vector
 * @param mesh Mesh data to collide against
 * @param transform Mesh world transform
 * @param outNormal Optional: receives collision normal if collision occurred
 * @return Actual movement applied (may be reduced/redirected by collision)
 */
R3DAPI Vector3 R3D_SlideCapsuleMesh(R3D_Capsule capsule, Vector3 velocity, R3D_MeshData mesh, Matrix transform, Vector3* outNormal);

/**
 * @brief Push sphere out of bounding box if penetrating
 * @param center Sphere center (modified in place if penetrating)
 * @param radius Sphere radius
 * @param box Obstacle box
 * @param outPenetration Optional: receives penetration depth
 * @return true if depenetration occurred
 */
R3DAPI bool R3D_DepenetrateSphereBoundingBox(Vector3* center, float radius, R3D_BoundingBox box, float* outPenetration);

/**
 * @brief Push capsule out of bounding box if penetrating
 * @param capsule Capsule shape (modified in place if penetrating)
 * @param box Obstacle box
 * @param outPenetration Optional: receives penetration depth
 * @return true if depenetration occurred
 */
R3DAPI bool R3D_DepenetrateCapsuleBoudningBox(R3D_Capsule* capsule, R3D_BoundingBox box, float* outPenetration);

/**
 * @brief Sweep sphere against single point
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Movement vector (direction and magnitude)
 * @param point Point to test against
 * @return Sweep collision info (hit, time, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepSpherePoint(Vector3 center, float radius, Vector3 velocity, Vector3 point);

/**
 * @brief Sweep sphere against line segment
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Movement vector (direction and magnitude)
 * @param a Segment start point
 * @param b Segment end point
 * @return Sweep collision info (hit, time, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepSphereSegment(Vector3 center, float radius, Vector3 velocity, Vector3 a, Vector3 b);

/**
 * @brief Sweep sphere against triangle plane (no edge/vertex clipping)
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Movement vector (direction and magnitude)
 * @param a Triangle vertex A
 * @param b Triangle vertex B
 * @param c Triangle vertex C
 * @return Sweep collision info (hit, time, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepSphereTrianglePlane(Vector3 center, float radius, Vector3 velocity, Vector3 a, Vector3 b, Vector3 c);

/**
 * @brief Sweep sphere against triangle with edge/vertex handling
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Movement vector (direction and magnitude)
 * @param a Triangle vertex A
 * @param b Triangle vertex B
 * @param c Triangle vertex C
 * @return Sweep collision info (hit, time, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepSphereTriangle(Vector3 center, float radius, Vector3 velocity, Vector3 a, Vector3 b, Vector3 c);

/**
 * @brief Sweep sphere along velocity vector
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Movement vector (direction and magnitude)
 * @param box Obstacle bounding box
 * @return Sweep collision info (hit, distance, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepSphereBoundingBox(Vector3 center, float radius, Vector3 velocity, R3D_BoundingBox box);

/**
 * @brief Sweep sphere along velocity vector against mesh geometry
 * @param center Sphere center position
 * @param radius Sphere radius
 * @param velocity Movement vector (direction and magnitude)
 * @param mesh Mesh data to test against
 * @param transform Mesh world transform
 * @return Sweep collision info (hit, time, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepSphereMesh(Vector3 center, float radius, Vector3 velocity, R3D_MeshData mesh, Matrix transform);

/**
 * @brief Sweep capsule along velocity vector
 * @param capsule Capsule shape to sweep
 * @param velocity Movement vector (direction and magnitude)
 * @param box Obstacle bounding box
 * @return Sweep collision info (hit, distance, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepCapsuleBoundingBox(R3D_Capsule capsule, Vector3 velocity, R3D_BoundingBox box);

/**
 * @brief Sweep capsule along velocity vector against mesh geometry
 * @param capsule Capsule shape to sweep
 * @param velocity Movement vector (direction and magnitude)
 * @param mesh Mesh data to test against
 * @param transform Mesh world transform
 * @return Sweep collision info (hit, time, point, normal)
 */
R3DAPI R3D_SweepCollision R3D_SweepCapsuleMesh(R3D_Capsule capsule, Vector3 velocity, R3D_MeshData mesh, Matrix transform);

/**
 * @brief Check if sphere is grounded against a box
 * @param center Sphere center
 * @param radius Sphere radius
 * @param checkDistance How far below to check
 * @param ground Ground box to test against
 * @param outGround Optional: receives raycast hit info
 * @return true if grounded within checkDistance
 */
R3DAPI bool R3D_IsSphereGroundedBoundingBox(Vector3 center, float radius, float checkDistance, R3D_BoundingBox ground, RayCollision* outGround);

/**
 * @brief Check if sphere is grounded against mesh geometry
 * @param center Sphere center
 * @param radius Sphere radius
 * @param checkDistance How far below to check
 * @param mesh Mesh data to test against
 * @param transform Mesh world transform
 * @param outGround Optional: receives raycast hit info
 * @return true if grounded within checkDistance
 */
R3DAPI bool R3D_IsSphereGroundedMesh(Vector3 center, float radius, float checkDistance, R3D_MeshData mesh, Matrix transform, RayCollision* outGround);

/**
 * @brief Check if capsule is grounded against a box
 * @param capsule Character capsule
 * @param checkDistance How far below to check (e.g., 0.1)
 * @param ground Ground box to test against
 * @param outGround Optional: receives raycast hit info
 * @return true if grounded within checkDistance
 */
R3DAPI bool R3D_IsCapsuleGroundedBoundingBox(R3D_Capsule capsule, float checkDistance, R3D_BoundingBox ground, RayCollision* outGround);

/**
 * @brief Check if capsule is grounded against mesh geometry
 * @param capsule Character capsule
 * @param checkDistance How far below to check
 * @param mesh Mesh data to test against
 * @param transform Mesh world transform
 * @param outGround Optional: receives raycast hit info
 * @return true if grounded within checkDistance
 */
R3DAPI bool R3D_IsCapsuleGroundedMesh(R3D_Capsule capsule, float checkDistance, R3D_MeshData mesh, Matrix transform, RayCollision* outGround);

#ifdef __cplusplus
} // extern "C"
#endif

/** @} */ // end of Kinematics

#endif // R3D_KINEMATICS_H
