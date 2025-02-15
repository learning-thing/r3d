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

#ifndef R3D_REGISTRY_H
#define R3D_REGISTRY_H

#include "./r3d_array.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    r3d_array_t elements;    // Stocke les objets directement (contigu)
    r3d_array_t valid_flags; // Stocke si un ID est valide (tableau booléen)
    r3d_array_t free_ids;    // Liste des IDs libérés
    unsigned int next_id;    // Prochain ID à attribuer
    size_t elem_size;        // Taille d'un élément
} r3d_registry_t;

static inline r3d_registry_t
r3d_registry_create(size_t capacity, size_t elem_size)
{
    r3d_registry_t registry = { 0 };
    registry.elements = r3d_array_create(capacity, elem_size);
    registry.valid_flags = r3d_array_create(capacity, sizeof(bool));
    registry.free_ids = r3d_array_create(capacity, sizeof(unsigned int));
    registry.next_id = 1;
    registry.elem_size = elem_size;
    return registry;
}

static inline void
r3d_registry_destroy(r3d_registry_t* registry)
{
    r3d_array_destroy(&registry->valid_flags);
    r3d_array_destroy(&registry->free_ids);
    r3d_array_destroy(&registry->elements);
}

static inline bool
r3d_registry_is_valid(r3d_registry_t* registry, unsigned int id)
{
    if (id == 0 || id >= registry->next_id) return false;
    return ((bool*)registry->valid_flags.data)[id - 1];
}

static inline unsigned int
r3d_registry_add(r3d_registry_t* registry, void* element)
{
    unsigned int id = 0;

    if (registry->free_ids.count > 0) {
        r3d_array_pop_back(&registry->free_ids, &id);
    }
    else {
        r3d_array_push_back(&registry->elements, NULL);
        r3d_array_push_back(&registry->valid_flags, &(bool){true});
        id = registry->next_id++;
    }

    void* elem = r3d_array_at(&registry->elements, id - 1);

    if (element) memcpy(elem, element, registry->elem_size);
    else memset(elem, 0, registry->elem_size);

    ((bool*)registry->valid_flags.data)[id - 1] = true;

    return id;
}

static inline void
r3d_registry_remove(r3d_registry_t* registry, unsigned int id)
{
    if (!r3d_registry_is_valid(registry, id)) return;

    r3d_array_push_back(&registry->free_ids, &id);
    ((bool*)registry->valid_flags.data)[id - 1] = false;
}

static inline void*
r3d_registry_get(r3d_registry_t* registry, unsigned int id)
{
    if (!r3d_registry_is_valid(registry, id)) return NULL;
    return r3d_array_at(&registry->elements, id - 1);
}

static inline unsigned int
r3d_registry_get_allocated_count(r3d_registry_t* registry)
{
    return (unsigned int)registry->elements.count;
}

#endif // R3D_REGISTRY_H
