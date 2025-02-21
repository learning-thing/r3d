#include "./r3d_collision.h"
#include <raymath.h>

bool r3d_collision_check_point_sphere(Vector3 point, Vector3 center, float radius)
{
    return Vector3Distance(point, center) < radius;
}

bool r3d_collision_check_point_cone(Vector3 point, Vector3 tip, Vector3 dir, float length, float radius)
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
