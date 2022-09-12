/**
 * @file 3d_object_factory.c
 * @author Pegasust (pegasucksgg@gmail.com)
 * @brief A source file for Object3D creations
 * @version 0.1
 * @date 2022-04-18
 * 
 */

#include <stdlib.h>
#include <stdint.h>
#include "3d.h"

typedef char *OrientationStrEnum;
// Needed because ORIENTATIONS need const eval
#define _FORWARD  "forward"
#define _BACKWARD "backward"
#define _UP       "up"
#define _DOWN     "down"
#define _LEFT     "left"
#define _RIGHT    "right"
// wrapper for type on enums
const OrientationStrEnum FORWARD  = _FORWARD  ;
const OrientationStrEnum BACKWARD = _BACKWARD ;
const OrientationStrEnum UP       = _UP       ;
const OrientationStrEnum DOWN     = _DOWN     ;
const OrientationStrEnum LEFT     = _LEFT     ;
const OrientationStrEnum RIGHT    = _RIGHT    ;
// orientation direction
const int POS_X = 0,
          POS_Y = 1,
          POS_Z = 2,
          NEG_X = 3,
          NEG_Y = 4,
          NEG_Z = 5,
          NO_MATCH = -1;
// axis
#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_Z  2

// effectively a translator. cartesian plane reference: 
// TODO: If it turns out my cartesian plane is wrong, edit here.
const OrientationStrEnum ORIENTATIONS[] = {
    _RIGHT,
    _UP,
    _BACKWARD,
    _LEFT,
    _DOWN,
    _FORWARD
};


int orientation_from_str(OrientationStrEnum orientation) {
    for(int i = 0; i < sizeof(ORIENTATIONS) / sizeof(ORIENTATIONS[0]); ++i) {
        if(!strcmp(orientation, ORIENTATIONS[i])) {
            // equals
            return i;
        }
    }
    return NO_MATCH;
}

int orientation_axis(int orientation) {
    if(orientation == NO_MATCH) {return orientation;}
    return orientation % 3;
}
int positive_direction(int orientation) {
    if(orientation == NO_MATCH) {return NO_MATCH;}
    return orientation < 3;
}

int orientation_match(OrientationStrEnum lhs, OrientationStrEnum rhs) {
    return !strcmp(lhs, rhs);
}
/**
 * @brief Merges by stealing root of `mover`. `mover` is effectively deallocated
 * 
 * @param merged 
 * @param mover 
 * @return Object3D* merged object that contains previously owned triangle
 * nodes from `mover`
 */
Object3D *Object3D_merge(Object3D* merged, Object3D** mover) {
    if(mover == NULL || *mover == NULL) {
        return merged;
    }
    Object3D *mov = *mover;
    // steal the mover's root
    // we would do the minimum count for least traversal
    // it's O(min(M,N))
    Object3D *traverse = (mov->count < merged->count)? mov: merged;
    Object3D *other = (Object3D*) (((uintptr_t)merged ^ (uintptr_t)mov) ^ (uintptr_t)traverse);
    Triangle3DNode **trav_end;
    for(trav_end = &traverse->root; *trav_end != NULL; trav_end = &((*trav_end)->next)) {
    }
    *trav_end = other->root;
    Triangle3DNode *new_node = traverse->root;
    merged->root = new_node;
    // assign new count
    merged->count += mov->count;
    // deallocate mover (no dtor on root because we "stole" its root)
    free(*mover);
    *mover = NULL;
    return merged;
}

double* axis_value(Coordinate3D* coord, int axis) {
    switch(axis) {
        case AXIS_X:
            return &coord->x;
        case AXIS_Y:
            return &coord->y;
        case AXIS_Z:
            return &coord->z;
    }
    return NULL;
}

// Object3D primitives
Object3D* Object3D_empty_ctor();
Object3D* Object3D_emplace_triangle(Object3D* obj, 
    Coordinate3D a, Coordinate3D b, Coordinate3D c);

// TODO: Fix these width/height functions if width-height detection not correct
double *_modify_width(Coordinate3D* origin, int axis) {
    switch(axis) {
        case AXIS_Z:
            return &origin->x;
        case AXIS_Y:
            return &origin->z;
        case AXIS_X:
            return &origin->y;
    }
    return NULL;
}
double *_modify_height(Coordinate3D* origin, int axis) {
    switch(axis) {
        case AXIS_Z:
            return &origin->y;
        case AXIS_Y:
            return &origin->x;
        case AXIS_X:
            return &origin->z;
    }
    return NULL;
}

struct RectangleCoords {
    Coordinate3D top_left;
    Coordinate3D top_right;
    Coordinate3D bot_right;
    Coordinate3D bot_left;
};

struct RectangleCoords* RectangleCoords_create(Coordinate3D origin, double width, double height, int axis) {
    double width_offset = width/2.0;
    double height_offset = height/2.0;
    Coordinate3D top_left = origin,
     top_right = origin,
     bot_right = origin, 
     bot_left = origin;
    if(axis == NO_MATCH) {
        return NULL;
    }
    *_modify_width(&top_left, axis)  -= width_offset;
    *_modify_height(&top_left, axis) += height_offset;

    *_modify_width(&top_right, axis) += width_offset;
    *_modify_height(&top_right, axis) += height_offset;

    *_modify_width(&bot_right, axis) += width_offset;
    *_modify_height(&bot_right, axis) -= height_offset;

    *_modify_width(&bot_left, axis) -= width_offset;
    *_modify_height(&bot_left, axis) -= height_offset;
    struct RectangleCoords* retval = malloc(sizeof(struct RectangleCoords));
    retval->top_left = top_left;
    retval->bot_left = bot_left;
    retval->bot_right = bot_right;
    retval->top_right = top_right;
    return retval;
}

Object3D *Object3D_create_rectangle(Coordinate3D origin, double width, double height, int axis) {

    Object3D *retval = Object3D_empty_ctor();
    struct RectangleCoords* c = RectangleCoords_create(origin, width, height, axis);

    Object3D_append_quadrilateral(retval, c->top_left, c->top_right, c->bot_right, c->bot_left);
    free(c);
    return retval;
}

// Object3D factories
Object3D *Object3D_create_pyramid(Coordinate3D origin, double width, double height, OrientationStrEnum orientation_str) {
    // create the 5 points of concerns:
    Coordinate3D pyramid_top = origin;
    // decode where the pyramid points to (OK!)
    int orientation = orientation_from_str(orientation_str);
    if(orientation == NO_MATCH) {
        return NULL;
    }
    int axis = orientation_axis(orientation);
    struct RectangleCoords * rect = RectangleCoords_create(origin, width, width, axis);
    // acquired the four points, now only need to calculate the top
    double* pyrtop_height_value = axis_value(&pyramid_top, axis);
    *pyrtop_height_value += (positive_direction(orientation)? height: -height);

    // emplace these points as triangles
    Object3D *pyramid = Object3D_empty_ctor();

    Object3D_append_quadrilateral(pyramid, rect->top_left, rect->top_right, rect->bot_right, rect->bot_left);

    // triangle parts
    Object3D_emplace_triangle(pyramid, rect->top_left, rect->top_right, pyramid_top);
    Object3D_emplace_triangle(pyramid, rect->bot_left, rect->bot_right, pyramid_top);
    Object3D_emplace_triangle(pyramid, rect->top_left, rect->bot_left, pyramid_top);
    Object3D_emplace_triangle(pyramid, rect->top_right, rect->bot_right, pyramid_top);
    free(rect);
    return pyramid;
}

Object3D *Object3D_create_cuboid(Coordinate3D origin, double width, double height, double depth) {
    // assemble 6 rectangles
    double w = width/2,
           h = height/2,
           d = depth/2;
    // bottom
    origin.y -= h;
    Object3D *temp;
    Object3D *cuboid = Object3D_create_rectangle(origin, depth, width, AXIS_Y);
    origin.y += h;
    // top
    origin.y += h;
    temp = Object3D_create_rectangle(origin, depth, width, AXIS_Y);
    cuboid = Object3D_merge(cuboid, &temp);
    origin.y -= h;
    // left
    origin.x -= w;
    temp = Object3D_create_rectangle(origin, height, depth, AXIS_X);
    cuboid = Object3D_merge(cuboid, &temp);
    origin.x += w;
    // right
    origin.x += w;
    temp = Object3D_create_rectangle(origin, height, depth, AXIS_X);
    cuboid = Object3D_merge(cuboid, &temp);
    origin.x -= w;
    // backwards
    origin.z -= d;
    temp = Object3D_create_rectangle(origin, width, height, AXIS_Z);
    cuboid = Object3D_merge(cuboid, &temp);
    origin.z += d;
    // forwards
    origin.z += d;
    temp = Object3D_create_rectangle(origin, width, height, AXIS_Z);
    cuboid = Object3D_merge(cuboid, &temp);
    origin.z -= d;
    return cuboid;
}
#define M_PI   3.14159265358979323846264338327950288
#define to_radians(degrees) ((degrees) * M_PI / 180.0)
void Coordinate3D_from_spherical_coord(Coordinate3D *out, const Coordinate3D * origin, double radius, double theta_deg, double phi_deg) {
    // Conforms the exact order with https://neutrium.net/mathematics/converting-between-spherical-and-cartesian-co-ordinate-systems/
    double phi = to_radians(phi_deg);
    double theta = to_radians(theta_deg);
    out->x = origin->x + (radius * sin(phi) * cos(theta));
    out->y = origin->y + (radius * sin(phi) * sin(theta));
    out->z = origin->z + (radius * cos(phi));
}

Object3D* Object3D_create_sphere(Coordinate3D origin, double radius, double increment) {
    Object3D *sphere = Object3D_empty_ctor();
    for(double phi = increment; phi <= 180.0; phi += increment) {
        for(double theta = 0; theta < 360.0; theta += increment) {
            // create the rect
            // the 4 points are given in the specs.
            Coordinate3D start_;
            Coordinate3D th_inc;
            Coordinate3D ph_inc;
            Coordinate3D next_s;

            Coordinate3D_from_spherical_coord(&start_, &origin, radius, theta, phi);
            Coordinate3D_from_spherical_coord(&th_inc, &origin, radius, theta, phi-increment);
            Coordinate3D_from_spherical_coord(&ph_inc, &origin, radius, theta-increment, phi);
            Coordinate3D_from_spherical_coord(&next_s, &origin, radius, theta-increment, phi-increment);

            Object3D_append_quadrilateral(sphere, start_, th_inc, ph_inc, next_s);
        }
    }

    return sphere;
}

#include <assert.h>
Object3D* Object3D_create_fractal(Coordinate3D origin, double size, int levels) {
    assert(levels >= 0 && "Negative levels, no eligible object.");
    if(levels == 0) {
        return Object3D_empty_ctor(); // nothing to return from
    }
    // start with one cube at origin
    Object3D* sponge = Object3D_create_cuboid(origin, size, size, size);
    if(levels == 1) {
        return sponge;
    }
    // merge with the 6 lower-level fractals
    double* mod_coords[6] = {
        &origin.x, &origin.x,
        &origin.y, &origin.y,
        &origin.z, &origin.z
    };
    double mod_amount[6] = {
        -size/2, size/2,
        -size/2, size/2,
        -size/2, size/2
    };
    for(int i = 0; i < 6; ++i) {
        *(mod_coords[i]) += mod_amount[i];
        Object3D *temp = Object3D_create_fractal(origin, size/2, levels-1);
        Object3D_merge(sponge, &temp);        
        *(mod_coords[i]) -= mod_amount[i];
    }
    return sponge;
}