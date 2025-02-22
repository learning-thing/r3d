#ifndef R3D_PROJECTION_H
#define R3D_PROJECTION_H

#include <raylib.h>

typedef struct {
    Vector2 position;
    bool inViewport;
    bool outNear;
    bool outFar;
} r3d_project_point_result_t;

r3d_project_point_result_t r3d_project_point(Vector3 point, Matrix viewProj, int screenWidth, int screenHeight);
Rectangle r3d_project_sphere_bounding_box(Vector3 center, float radius, Vector3 viewPos, Matrix viewProj, int screenWidth, int screenHeight);
Rectangle r3d_project_cone_bounding_box(Vector3 tip, Vector3 dir, float length, float cosTheta, Vector3 viewPos, Matrix viewProj, int screenWidth, int screenHeight);

#endif // R3D_PROJECTION_H
