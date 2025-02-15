/*
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./r3d_primitives.h"

r3d_primitive_t r3d_primitive_load_quad(void)
{
    static const float VERTICES[] =
    {
        // Positions         Normals             Texcoords
       -1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    };

    static const unsigned short INDICES[] =
    {
        0, 1, 2,  // First triangle (bottom-left, bottom-right, top-left)
        1, 3, 2   // Second triangle (bottom-right, top-right, top-left)
    };

    r3d_primitive_t quad = { 0 };

    quad.vao = rlLoadVertexArray();
    rlEnableVertexArray(quad.vao);

    quad.ebo = rlLoadVertexBufferElement(INDICES, sizeof(INDICES), false);
    quad.vbo = rlLoadVertexBuffer(VERTICES, sizeof(VERTICES), false);

    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, false, 8 * sizeof(float), 0);
    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));
    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));

    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);

    rlDisableVertexArray();

    return quad;
}

r3d_primitive_t r3d_primitive_load_cube(void)
{
    static const float VERTICES[] =
    {
        // Positions            // Normals             // Texcoords
       -1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f,  // Front top-left
       -1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,  // Front bottom-left
        1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f,  // Front top-right
        1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,  // Front bottom-right

       -1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,  // Back top-left
       -1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,  // Back bottom-left
        1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f,  // Back top-right
        1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,  // Back bottom-right
    };

    static const unsigned short INDICES[] =
    {
        // Front face
        0, 1, 2, 2, 1, 3,
        // Back face
        4, 5, 6, 6, 5, 7,
        // Left face
        4, 5, 0, 0, 5, 1,
        // Right face
        2, 3, 6, 6, 3, 7,
        // Top face
        4, 0, 6, 6, 0, 2,
        // Bottom face
        1, 5, 3, 3, 5, 7
    };

    r3d_primitive_t cube = { 0 };

    cube.vao = rlLoadVertexArray();
    rlEnableVertexArray(cube.vao);

    cube.ebo = rlLoadVertexBufferElement(INDICES, sizeof(INDICES), false);
    cube.vbo = rlLoadVertexBuffer(VERTICES, sizeof(VERTICES), false);

    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, false, 8 * sizeof(float), 0);
    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));
    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));

    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);

    rlDisableVertexArray();

    return cube;
}

void r3d_primitive_unload(r3d_primitive_t* primitive)
{
    rlUnloadVertexBuffer(primitive->vbo);
    rlUnloadVertexBuffer(primitive->ebo);
    rlUnloadVertexArray(primitive->vao);
}

void r3d_primitive_draw(r3d_primitive_t* primitive)
{
    bool vaoEnabled = rlEnableVertexArray(primitive->vao);

    if (!vaoEnabled) {
        rlEnableVertexBuffer(primitive->vbo);

        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);

        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);

        rlEnableVertexBufferElement(primitive->ebo);
    }

    rlDrawVertexArrayElements(0, 36, 0);

    if (vaoEnabled) {
        rlDisableVertexArray();
    }
    else {
        rlDisableVertexBuffer();
        rlDisableVertexBufferElement();
    }
}
