/* r3d_shape.h -- R3D Shape Module.
 *
 * Copyright (c) 2025-2026 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef R3D_SHAPE_H
#define R3D_SHAPE_H

#include "./r3d_mesh_data.h"
#include "./r3d_model.h"

/**
 * @defgroup Shape
 * @brief Collision shape types and read-only queries (construction, intersection, penetration, closest points).
 *
 * Functions that modify a shape's state belong to the Kinematics module.
 * @{
 */

// ========================================
// STRUCTS TYPES
// ========================================

/**
 * @brief Alias for raylib's BoundingBox (min/max corner points).
 */
typedef BoundingBox R3D_BoundingBox;

/**
 * @brief Oriented bounding box (OBB).
 *
 * Defined by a center point, three orthogonal axes, and half-extents along each axis.
 */
typedef struct {
    Vector3 center;
    Vector3 axisX;
    Vector3 axisY;
    Vector3 axisZ;
    Vector3 halfExtents;
} R3D_OrientedBox;

/**
 * @brief Capsule shape defined by two endpoints and radius
 */
typedef struct R3D_Capsule {
    Vector3 start;      ///< Start point of capsule axis
    Vector3 end;        ///< End point of capsule axis
    float radius;       ///< Capsule radius
} R3D_Capsule;

/**
 * @brief Penetration information from an overlap test
 */
typedef struct R3D_Penetration {
    bool collides;      ///< Whether shapes are overlapping
    float depth;        ///< Penetration depth
    Vector3 normal;     ///< Collision normal (direction to resolve penetration)
    Vector3 mtv;        ///< Minimum Translation Vector (normal * depth)
} R3D_Penetration;

// ========================================
// PUBLIC API
// ========================================

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compute an axis-aligned bounding box from a center and half-extents.
 */
R3DAPI R3D_BoundingBox R3D_GetBoundingBox(Vector3 center, Vector3 halfExtents);

/**
 * @brief Compute an oriented bounding box from an AABB and transform.
 */
R3DAPI R3D_OrientedBox R3D_GetOrientedBox(BoundingBox aabb, Matrix transform);

/**
 * @brief Check if capsule intersects with box
 * @param capsule Capsule shape
 * @param box Bounding box
 * @return true if collision detected
 */
R3DAPI bool R3D_CheckCollisionCapsuleBox(R3D_Capsule capsule, BoundingBox box);

/**
 * @brief Check if capsule intersects with sphere
 * @param capsule Capsule shape
 * @param center Sphere center
 * @param radius Sphere radius
 * @return true if collision detected
 */
R3DAPI bool R3D_CheckCollisionCapsuleSphere(R3D_Capsule capsule, Vector3 center, float radius);

/**
 * @brief Check if two capsules intersect
 * @param a First capsule
 * @param b Second capsule
 * @return true if collision detected
 */
R3DAPI bool R3D_CheckCollisionCapsules(R3D_Capsule a, R3D_Capsule b);

/**
 * @brief Check if capsule intersects with mesh
 * @param capsule Capsule shape
 * @param mesh Mesh data
 * @param transform Mesh transform
 * @return true if collision detected
 */
R3DAPI bool R3D_CheckCollisionCapsuleMesh(R3D_Capsule capsule, R3D_MeshData mesh, Matrix transform);

/**
 * @brief Check penetration between capsule and box
 * @param capsule Capsule shape
 * @param box Bounding box
 * @return Penetration information.
 */
R3DAPI R3D_Penetration R3D_CheckPenetrationCapsuleBox(R3D_Capsule capsule, BoundingBox box);

/**
 * @brief Check penetration between capsule and sphere
 * @param capsule Capsule shape
 * @param center Sphere center
 * @param radius Sphere radius
 * @return Penetration information.
 */
R3DAPI R3D_Penetration R3D_CheckPenetrationCapsuleSphere(R3D_Capsule capsule, Vector3 center, float radius);

/**
 * @brief Check penetration between two capsules
 * @param a First capsule
 * @param b Second capsule
 * @return Penetration information.
 */
R3DAPI R3D_Penetration R3D_CheckPenetrationCapsules(R3D_Capsule a, R3D_Capsule b);

/**
 * @brief Cast a ray against mesh geometry
 * @param ray Ray to cast
 * @param mesh Mesh data to test against
 * @param transform Mesh world transform
 * @return Ray collision info (hit, distance, point, normal)
 */
R3DAPI RayCollision R3D_RaycastMesh(Ray ray, R3D_MeshData mesh, Matrix transform);

/**
 * @brief Cast a ray against a model (tests all meshes)
 * @param ray Ray to cast
 * @param model Model to test against (must have valid meshData)
 * @param transform Model world transform
 * @return Ray collision info for closest hit (hit=false if no meshData)
 */
R3DAPI RayCollision R3D_RaycastModel(Ray ray, R3D_Model model, Matrix transform);

/**
 * @brief Find closest point on line segment to given point
 * @param point Query point
 * @param start Segment start
 * @param end Segment end
 * @return Closest point on segment [start, end]
 */
R3DAPI Vector3 R3D_ClosestPointOnSegment(Vector3 point, Vector3 start, Vector3 end);

/**
 * @brief Find closest point on triangle to given point
 * @param p Query point
 * @param a Triangle vertex A
 * @param b Triangle vertex B
 * @param c Triangle vertex C
 * @return Closest point on triangle surface
 */
R3DAPI Vector3 R3D_ClosestPointOnTriangle(Vector3 p, Vector3 a, Vector3 b, Vector3 c);

/**
 * @brief Find closest point on box surface to given point
 * @param point Query point
 * @param box Bounding box
 * @return Closest point on/in box (clamped to box bounds)
 */
R3DAPI Vector3 R3D_ClosestPointOnBox(Vector3 point, BoundingBox box);

#ifdef __cplusplus
} // extern "C"
#endif

/** @} */ // end of Shape

#endif // R3D_SHAPE_H
