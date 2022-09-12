/**
 * @file generator.c
 * @author Pegasust
 * @brief A driver for 3d.o
 * @version 0.1
 * @date 2022-04-19
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "3d.h"

void serialize(Scene3D* scene, char* filename) {
    int flen = strlen(filename);
    char *f_extended = malloc(flen + sizeof(".bin.stl"));
    strncpy(f_extended, filename, flen);
    strcpy(&f_extended[flen], ".stl");
    Scene3D_write_stl_text(scene, f_extended);
    printf("Wrote to %s\n", f_extended);
    strcpy(&f_extended[flen], ".bin");
    strcpy(&f_extended[strlen(f_extended)], ".stl");
    Scene3D_write_stl_binary(scene, f_extended);
    printf("Wrote to %s\n", f_extended);
    free(f_extended);
}

int main() {
    // create star
    Scene3D *star = Scene3D_create();
    char* directions[] = {"up", "down", "left", "right", "forward", "backward"};
    Coordinate3D origin = {100, 100, 100};
    for(int i = 0; i < 6; ++i) {
        Object3D *object = Object3D_create_pyramid(origin, 20, 30, directions[i]);
        Scene3D_append(star, object);
    }
    serialize(star, "star");
    Scene3D_destroy(star);

    // create robo-lookalike
    Coordinate3D coord;
    Object3D *object;
    Scene3D* face = Scene3D_create();
    // head
    coord = (Coordinate3D){25, 25, 25};
    object = Object3D_create_cuboid(coord, 50, 50, 50);
    Scene3D_append(face, object);
    // eye
    coord = (Coordinate3D){15, 40, 0};
    object = Object3D_create_cuboid(coord, 10, 10, 10);
    Scene3D_append(face, object);

    // eye
    coord = (Coordinate3D){35, 40, 0};
    object = Object3D_create_cuboid(coord, 10, 10, 10);
    Scene3D_append(face, object);
    // mouth
    coord = (Coordinate3D){25, 15, 0};
    object = Object3D_create_cuboid(coord, 30, 7, 10);
    Scene3D_append(face, object);
    serialize(face, "face");
    Scene3D_destroy(face);

    // pyramid
    Scene3D *pyramid = Scene3D_create();
    object = Object3D_create_pyramid((Coordinate3D){0,0,0},50, 75, "up");
    Scene3D_append(pyramid, object);

    serialize(pyramid, "pyramid");

    Scene3D_destroy(pyramid);

    // sphere
    Scene3D *sphere = Scene3D_create();
    object = Object3D_create_sphere((Coordinate3D){0, 0, 0}, 20, 90);
    Scene3D_append(sphere, object);

    serialize(sphere, "sphere");
    Scene3D_destroy(sphere);

    // spheres
    Scene3D *spheres = Scene3D_create();
    double inc[] = {15, 10, 5, 36, 30, 20, 90, 60, 45};
    for(int i = 0; i < 3; i += 1) {
        for(int j = 0; j < 3; ++j) {
            object = Object3D_create_sphere((Coordinate3D){j*100, i*100, 0}, 45, inc[(i*3)+j]);
            Scene3D_append(spheres, object);
            printf("count: %ld\n", object->count);
        }
    }
    serialize(spheres, "spheres");
    Scene3D_destroy(spheres);


    // fractals
    Scene3D* fractals = Scene3D_create();
    const int highest_level = 6;

    for(int level = 0; level <= highest_level; ++level) {
        int i = level / 3;
        int j = level % 3;
        Coordinate3D origin = (Coordinate3D){j*100, i*100, 0};
        object = Object3D_create_fractal(origin, 50, level);
        Scene3D_append(fractals, object);
        printf("Count for level %d: %ld\n", level, object->count);
    }
    
    serialize(fractals, "fractals");
    Scene3D_destroy(fractals);
    printf("All written!\n");
    return 0;
}