/**
 * @file 3d_writer.c
 * @author Pegasust
 * @brief 
 * @version 0.1
 * @date 2022-04-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "3d.h"

void Scene3D_fwrite_stl_text(Scene3D* scene, FILE* f) {
    fprintf(f, "solid scene\n");
    for(int i = 0; i < scene->count; ++i) {
        Object3D* object = scene->objects[i];
        int j = 0;
        for(Triangle3DNode* tri_iter = object->root; tri_iter != NULL; tri_iter = tri_iter->next) {
            fprintf(f, "  facet normal 0.0 0.0 0.0\n");
            fprintf(f, "    outer loop\n");
            ++j;
            Coordinate3D* tris[3] = {
                &tri_iter->triangle.a,
                &tri_iter->triangle.b,
                &tri_iter->triangle.c
            };
            for(int e = 0; e < 3; ++e) {
                fprintf(f, "    vertex %.5f %.5f %.5f\n",
                    tris[e]->x, tris[e]->y, tris[e]->z);
            }
            fprintf(f, "    endloop\n");
            fprintf(f, "  endfacet\n");
        }
        if(j != object->count) {
            fprintf(stderr, "j (%d) != count (%ld)\n", j, object->count);
        }
    }
    fprintf(f, "endsolid scene\n");
}

const uint8_t* get_header() {
    static uint8_t header[80];
    static uint8_t initialized = 0;
    if(!initialized) {
        memset(header, 0, 80);
        initialized = 1;
    }
    return header;
}

void Scene3D_fwrite_stl_binary(Scene3D* scene, FILE* f) {
    // first 80 bytes: header
    // should not begin with "solid"
    // can be anything
    fwrite(get_header(), sizeof(uint8_t), 80, f);

    // facet count (uint32_t)
    uint32_t facet_count = 0;
    for(uint32_t i = 0; i < scene->count; ++i) {
        facet_count += scene->objects[i]->count;
    }
    fwrite(&facet_count, sizeof(uint32_t), 1, f);

    // the facets, each is 50 bytes
    for(uint32_t facets_i = 0; facets_i < scene->count; ++facets_i) {
        for(Triangle3DNode* iter = scene->objects[facets_i]->root; iter != NULL; iter = iter->next) {
            // 12 bytes normal: 3x4-byte FP (float)
            // not supported by the data structure, so it's all 0
            float norms[] = {0.0f, 0.0f, 0.0f};
            fwrite(&norms, sizeof(norms[0]), sizeof(norms)/sizeof(norms[0]), f);
            Coordinate3D* tris[3] = {
                &iter->triangle.a,
                &iter->triangle.b,
                &iter->triangle.c
            };
            // 9x4-byte floats: coordinates of corners
            for(int i = 0; i < 3; ++i) {
                norms[0] = tris[i]->x;
                norms[1] = tris[i]->y;
                norms[2] = tris[i]->z;
                fwrite(&norms, sizeof(norms[0]), sizeof(norms)/sizeof(norms[0]), f);
            }
            // attribute: not supported, just 0 for now
            uint16_t attribute = 0;
            fwrite(&attribute, sizeof(attribute), 1, f);
        }
    }
}

void Scene3D_write_stl_text(Scene3D* scene, char* file_name) {
    FILE* f = fopen(file_name, "w");
    Scene3D_fwrite_stl_text(scene, f);
    fclose(f);
}

void Scene3D_write_stl_binary(Scene3D* scene, char* file_name) {
    FILE* f = fopen(file_name, "wb");
    Scene3D_fwrite_stl_binary(scene, f);
    fclose(f);
}