#ifndef R3D_DETAIL_BILLBOARD_H
#define R3D_DETAIL_BILLBOARD_H

#include <raylib.h>

void r3d_billboard_mode_front(Matrix* model, const Matrix* invView);
void r3d_billboard_mode_y(Matrix* model, const Matrix* invView);

#endif // R3D_DETAIL_BILLBOARD_H
