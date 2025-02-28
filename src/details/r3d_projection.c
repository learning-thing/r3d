#include "./r3d_projection.h"
#include "./r3d_collision.h"
#include <raymath.h>
#include <float.h>

r3d_project_point_result_t r3d_project_point(Vector3 point, Matrix viewProj, int screenWidth, int screenHeight)
{
    r3d_project_point_result_t result = { 0 };

    // Transform the 3D point into homogeneous clip space coordinates
    Vector4 clipSpace;
    clipSpace.x = point.x * viewProj.m0 + point.y * viewProj.m4 + point.z * viewProj.m8 + viewProj.m12;
    clipSpace.y = point.x * viewProj.m1 + point.y * viewProj.m5 + point.z * viewProj.m9 + viewProj.m13;
    clipSpace.z = point.x * viewProj.m2 + point.y * viewProj.m6 + point.z * viewProj.m10 + viewProj.m14;
    clipSpace.w = point.x * viewProj.m3 + point.y * viewProj.m7 + point.z * viewProj.m11 + viewProj.m15;

    // Checks if the point is behind the near plane  
    result.outNear = (clipSpace.w <= 0.0);

    // Checks if the point is beyond the far plane  
    result.outFar = (clipSpace.z > clipSpace.w);

    // Compute the screen coordinates even if the point is not visible
    // Divide by w to convert to Normalized Device Coordinates (NDC)
    float invW = 1.0f / clipSpace.w;
    float ndcX = clipSpace.x * invW;
    float ndcY = clipSpace.y * invW;

    // Determine if the point is within the viewport bounds
    result.inViewport = (ndcX >= -1.0f && ndcX <= 1.0f && ndcY >= -1.0f && ndcY <= 1.0f);

    // Convert NDC to screen space coordinates
    result.position.x = (ndcX + 1.0f) * 0.5f * screenWidth;
    result.position.y = (1.0f - (ndcY + 1.0f) * 0.5f) * screenHeight;

    return result;
}

Rectangle r3d_project_sphere_bounding_box(Vector3 center, float radius, Vector3 viewPos, Matrix viewProj, int screenWidth, int screenHeight)
{
    Rectangle boundingBox = { 0 };

    // If the camera is inside the projected sphere, assume the entire screen is affected.
    // This is not entirely accurate, but the result would be the same if we performed
    // the full projection, with a potential additional margin of error.
    if (r3d_collision_check_point_in_sphere(viewPos, center, radius)) {
        boundingBox.width = (float)screenWidth;
        boundingBox.height = (float)screenHeight;
        return boundingBox;
    }

    // Create 8 points representing the corners of a cube that encloses the sphere.
    Vector3 points[8];
    points[0] = (Vector3) { center.x - radius, center.y - radius, center.z - radius };
    points[1] = (Vector3) { center.x + radius, center.y - radius, center.z - radius };
    points[2] = (Vector3) { center.x - radius, center.y + radius, center.z - radius };
    points[3] = (Vector3) { center.x + radius, center.y + radius, center.z - radius };
    points[4] = (Vector3) { center.x - radius, center.y - radius, center.z + radius };
    points[5] = (Vector3) { center.x + radius, center.y - radius, center.z + radius };
    points[6] = (Vector3) { center.x - radius, center.y + radius, center.z + radius };
    points[7] = (Vector3) { center.x + radius, center.y + radius, center.z + radius };

    // Initialize min/max values for computing the bounding rectangle.
    float minX = (float)screenWidth;
    float minY = (float)screenHeight;
    float maxX = 0, maxY = 0;

    // Project each point and determine the min/max screen coordinates.
    for (int i = 0; i < 8; i++) {
        r3d_project_point_result_t result = r3d_project_point(
            points[i], viewProj, screenWidth, screenHeight
        );

        // Ignore points that are behind the near plane.
        if (result.outNear) continue;

        if (result.position.x < minX) minX = result.position.x;
        if (result.position.x > maxX) maxX = result.position.x;
        if (result.position.y < minY) minY = result.position.y;
        if (result.position.y > maxY) maxY = result.position.y;
    }

    // Construct the bounding rectangle using the computed min/max values.
    boundingBox.x = minX;
    boundingBox.y = minY;
    boundingBox.width = maxX - minX;
    boundingBox.height = maxY - minY;

    return boundingBox;
}

Rectangle r3d_project_cone_bounding_box(Vector3 tip, Vector3 dir, float length, float radius, Vector3 viewPos, Matrix viewProj, int screenWidth, int screenHeight)
{
    Rectangle boundingBox = { 0 };

    // If the camera is inside the projected cone, assume the entire screen is affected.
    // This is not entirely accurate, but the result would be the same if we performed
    // the full projection, with a potential additional margin of error.
    if (r3d_collision_check_point_in_cone(viewPos, tip, dir, length, radius)) {
        boundingBox.width = (float)screenWidth;
        boundingBox.height = (float)screenHeight;
        return boundingBox;
    }

    // Compute the position of the cone's base.
    Vector3 base = {
        tip.x + dir.x * length,
        tip.y + dir.y * length,
        tip.z + dir.z * length
    };

    // Find two perpendicular vectors to the direction vector.
    Vector3 right = { 0 };
    Vector3 up = { 0 };

    // Find a vector perpendicular to dir.
    if (fabsf(dir.x) < fabsf(dir.y) && fabsf(dir.x) < fabsf(dir.z)) {
        right = (Vector3){ 0, -dir.z, dir.y };
    }
    else if (fabsf(dir.y) < fabsf(dir.z)) {
        right = (Vector3){ -dir.z, 0, dir.x };
    }
    else {
        right = (Vector3){ -dir.y, dir.x, 0 };
    }

    // Normalize the right vector.
    float rightLen = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
    right.x /= rightLen;
    right.y /= rightLen;
    right.z /= rightLen;

    // Compute the up vector (perpendicular to dir and right).
    up.x = dir.y * right.z - dir.z * right.y;
    up.y = dir.z * right.x - dir.x * right.z;
    up.z = dir.x * right.y - dir.y * right.x;

    // Define the points of the cone to be projected.
    Vector3 points[9];
    points[0] = tip;  // Cone's apex.

    // Generate 8 evenly spaced points around the base circle.
    for (int i = 0; i < 8; i++) {
        float angle = i * (2.0f * PI / 8.0f);
        float cosA = cosf(angle);
        float sinA = sinf(angle);

        points[i + 1] = (Vector3){
            base.x + radius * (cosA * right.x + sinA * up.x),
            base.y + radius * (cosA * right.y + sinA * up.y),
            base.z + radius * (cosA * right.z + sinA * up.z)
        };
    }

    // Initialize min/max values for computing the bounding rectangle.
    float minX = (float)screenWidth;
    float minY = (float)screenHeight;
    float maxX = 0, maxY = 0;

    // Project each point and determine the min/max screen coordinates.
    for (int i = 0; i < 9; i++) {
        r3d_project_point_result_t result = r3d_project_point(
            points[i], viewProj, screenWidth, screenHeight
        );

        // Ignore points that are behind the near plane.
        if (result.outNear) continue;

        if (result.position.x < minX) minX = result.position.x;
        if (result.position.x > maxX) maxX = result.position.x;
        if (result.position.y < minY) minY = result.position.y;
        if (result.position.y > maxY) maxY = result.position.y;
    }

    // Construct the bounding rectangle using the computed min/max values.
    boundingBox.x = minX;
    boundingBox.y = minY;
    boundingBox.width = maxX - minX;
    boundingBox.height = maxY - minY;

    return boundingBox;
}
