#include "./r3d_collision.h"
#include <raymath.h>

bool r3d_collision_check_point_in_sphere(Vector3 point, Vector3 center, float radius)
{
    return Vector3Distance(point, center) < radius;
}

bool r3d_collision_check_point_in_sphere_sqr(Vector3 point, Vector3 center, float radius)
{
    return Vector3DistanceSqr(point, center) < radius * radius;
}

bool r3d_collision_check_sphere_in_sphere(Vector3 c1, float r1, Vector3 c2, float r2)
{
    // Two spheres intersect if the distance between their centers
    // is less than the sum of their radii
    return Vector3Distance(c1, c2) < (r1 + r2);
}

bool r3d_collision_check_sphere_in_sphere_sqr(Vector3 c1, float r1, Vector3 c2, float r2)
{
    // Same as above but using squared distance to avoid square root calculation
    float radiusSum = r1 + r2;
    return Vector3DistanceSqr(c1, c2) < (radiusSum * radiusSum);
}

bool r3d_collision_check_point_in_cone(Vector3 point, Vector3 tip, Vector3 dir, float length, float radius)
{
    // Check if the point is between the tip and the base of the cone.
    Vector3 tipToPoint = Vector3Subtract(point, tip);
    float projectionOnAxis = Vector3DotProduct(tipToPoint, dir);

    // If the point is outside the cone's height range, return false.
    if (projectionOnAxis <= 0 || projectionOnAxis >= length) {
        return false;
    }

    // Compute the perpendicular distance from the point to the cone's axis.
    Vector3 projectionVector = Vector3Scale(dir, projectionOnAxis);
    Vector3 perpVector = Vector3Subtract(tipToPoint, projectionVector);
    float perpDistance = Vector3Length(perpVector);

    // Compute the cone's radius at the point's height along the axis.
    float coneRadiusAtPoint = (projectionOnAxis / length) * radius;

    // The point is inside the cone if it is within the computed radius.
    return perpDistance < coneRadiusAtPoint;
}

bool r3d_collision_check_sphere_in_cone(Vector3 sCenter, float sRadius, Vector3 cTip, Vector3 cDir, float cLength, float cRadius)
{
    // First check if any part of the sphere intersects the cone's height range
    Vector3 tipToCenter = Vector3Subtract(sCenter, cTip);
    float projectionOnAxis = Vector3DotProduct(tipToCenter, cDir);

    // If the sphere is completely before the tip or after the base, no collision
    if (projectionOnAxis + sRadius < 0 || projectionOnAxis - sRadius > cLength) {
        return false;
    }

    // Calculate perpendicular distance from sphere center to cone axis
    Vector3 projectionVector = Vector3Scale(cDir, projectionOnAxis);
    Vector3 perpVector = Vector3Subtract(tipToCenter, projectionVector);
    float perpDistance = Vector3Length(perpVector);

    // For each relevant point along the height of the sphere, check if it's within the cone
    // We only need to check the closest and farthest points of the sphere
    float minHeight = fmaxf(0, projectionOnAxis - sRadius);
    float maxHeight = fminf(cLength, projectionOnAxis + sRadius);

    // Calculate cone radii at these heights
    float minRadius = (minHeight / cLength) * cRadius;
    float maxRadius = (maxHeight / cLength) * cRadius;

    // The sphere intersects the cone if its center is within the larger cone defined by
    // adding the sphere's radius to the cone's radius at each point
    return perpDistance <= maxRadius + sRadius;
}
