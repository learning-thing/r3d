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

#ifndef R3D_DETAILS_DDS_LOADER_EXT_H
#define R3D_DETAILS_DDS_LOADER_EXT_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
    Extended DDS loader for RG16F and RG32F formats.
    Supports:
    - Uncompressed DDS files in RG16F (111) or RG32F (112) DXGI formats
    - Base level only (no mipmaps)
    - Proper format detection using DXGI format codes
*/

#define FOURCC_DDS     0x20534444  // "DDS " in little-endian
#define FOURCC_DX10    0x30315844  // "DX10" format extension header
#define DDPF_FOURCC    0x4         // Contains FourCC code

// DDS pixel format structure
typedef struct {
    uint32_t size;
    uint32_t flags;
    uint32_t fourcc;
    uint32_t rgb_bit_count;
    uint32_t r_bit_mask;
    uint32_t g_bit_mask;
    uint32_t b_bit_mask;
    uint32_t a_bit_mask;
} dds_pixelformat;

// Main DDS header structure
typedef struct {
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitch_or_linear_size;
    uint32_t depth;
    uint32_t mipmap_count;
    uint32_t reserved1[11];
    dds_pixelformat ddspf;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
} dds_header;

// DX10 header extension
typedef struct {
    uint32_t dxgi_format;
    uint32_t resource_dimension;
    uint32_t misc_flag;
    uint32_t array_size;
    uint32_t misc_flags2;
} dds_header_dx10;

// DXGI format codes for the formats we support
#define DXGI_FORMAT_R16G16_FLOAT    111
#define DXGI_FORMAT_R32G32_FLOAT    112

/*
    Loads an uncompressed DDS texture in RG16F or RG32F format into memory.
    Parameters:
        - file_data: Pointer to the DDS file data in memory
        - file_size: Size of the file in bytes
        - width: Pointer to store the texture width
        - height: Pointer to store the texture height
        - format_size: Pointer to store the format size (2 for RG16F, 4 for RG32F)
    Returns:
        - Pointer to the raw image data if successful
        - NULL if the file is invalid or format not supported
*/
static inline void* r3d_load_dds_from_memory_ext(
    const unsigned char* file_data,
    size_t file_size,
    uint32_t* width,
    uint32_t* height,
    uint32_t* format_size
) {
    // Basic validation
    if (!file_data || !width || !height || !format_size ||
        file_size < sizeof(dds_header) + 4) {
        return NULL;
    }

    // Check DDS magic number
    const uint32_t magic = *(const uint32_t*)file_data;
    if (magic != FOURCC_DDS) {
        return NULL;
    }

    // Read the main header
    const dds_header* header = (const dds_header*)(file_data + 4);
    
    // Validate header size
    if (header->size != sizeof(dds_header)) {
        return NULL;
    }

    uint32_t dxgi_format = 0;
    size_t data_offset = 4 + sizeof(dds_header);

    // Check if we have a DX10 header
    if ((header->ddspf.flags & DDPF_FOURCC) &&
        header->ddspf.fourcc == FOURCC_DX10) {
        // Validate file size for DX10 header
        if (file_size < data_offset + sizeof(dds_header_dx10)) {
            return NULL;
        }

        // Read DX10 header
        const dds_header_dx10* dx10_header =
            (const dds_header_dx10*)(file_data + data_offset);
        dxgi_format = dx10_header->dxgi_format;
        data_offset += sizeof(dds_header_dx10);
    }
    else {
        // Check standard DDS format based on bit masks
        if (header->ddspf.rgb_bit_count == 32 && 
            header->ddspf.r_bit_mask == 0x0000FFFF && 
            header->ddspf.g_bit_mask == 0xFFFF0000) {
            dxgi_format = DXGI_FORMAT_R16G16_FLOAT;
        }
        //else if (header->ddspf.rgb_bit_count == 64 && 
        //         header->ddspf.r_bit_mask == 0xFFFFFFFF && 
        //         header->ddspf.g_bit_mask == 0xFFFFFFFF00000000ULL) {
        //    dxgi_format = DXGI_FORMAT_R32G32_FLOAT;
        //}
    }

    // Determine format and size
    if (dxgi_format == DXGI_FORMAT_R16G16_FLOAT) {
        *format_size = 4;  // 2 channels * 2 bytes
    }
    else if (dxgi_format == DXGI_FORMAT_R32G32_FLOAT) {
        *format_size = 8;  // 2 channels * 4 bytes
    }
    else {
        return NULL;  // Unsupported format
    }

    *width = header->width;
    *height = header->height;

    // Compute and validate data size
    const size_t data_size = (size_t)header->width * header->height * (*format_size);
    if (file_size < data_offset + data_size) {
        return NULL;
    }

    // Allocate and copy image data
    void* image_data = RL_MALLOC(data_size);
    if (!image_data) {
        return NULL;
    }

    memcpy(image_data, file_data + data_offset, data_size);
    return image_data;
}

#endif // R3D_DETAILS_DDS_LOADER_EXT_H
