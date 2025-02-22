#ifndef R3D_COLLISION_H
#define R3D_COLLISION_H

#include <raylib.h>

bool r3d_collision_check_point_in_sphere(Vector3 point, Vector3 center, float radius);
bool r3d_collision_check_point_in_sphere_sqr(Vector3 point, Vector3 center, float radius);

bool r3d_collision_check_sphere_in_sphere(Vector3 c1, float r1, Vector3 c2, float r2);
bool r3d_collision_check_sphere_in_sphere_sqr(Vector3 c1, float r1, Vector3 c2, float r2);

bool r3d_collision_check_point_in_cone(Vector3 point, Vector3 tip, Vector3 dir, float length, float radius);
bool r3d_collision_check_sphere_in_cone(Vector3 sCenter, float sRadius, Vector3 cTip, Vector3 cDir, float cLength, float cRadius);

#endif // R3D_COLLISION_H
