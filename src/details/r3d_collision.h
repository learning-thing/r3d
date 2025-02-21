#ifndef R3D_COLLISION_H
#define R3D_COLLISION_H

#include <raylib.h>

bool r3d_collision_check_point_sphere(Vector3 point, Vector3 center, float radius);
bool r3d_collision_check_point_cone(Vector3 point, Vector3 tip, Vector3 dir, float length, float radius);

#endif // R3D_COLLISION_H
