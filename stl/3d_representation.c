/**
 * @file 3d_representation.c
 * @author Pegasust
 * @brief A source file for dealing with methods of
 * 3D triangle-based concerns
 * @version 0.1
 * @date 2022-04-18
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "3d.h"

#define ARRAYLIST_OBJECTS_INITIAL_CAPACITY (1024/sizeof(void*))

Object3D* Object3D_empty_ctor() {
    Object3D* retval = malloc(sizeof(Object3D));
    retval->count = 0;
    retval->root = NULL;

    return retval;
}

/**
 * @brief Pushes a new triangle node in front of `obj->objects`
 * 
 * @param obj 
 * @param node 
 * @return Object3D* 
 */
Object3D* Object3D_push_triangle_node(Object3D* obj, Triangle3DNode* node) {
    if(node == NULL) {return obj;}
    ++obj->count;
    node->next = obj->root;
    obj->root = node;
    return obj;
}
/**
 * @brief Adds a new triangle by construction to obj
 * 
 * @param obj 
 * @param a 
 * @param b 
 * @param c 
 * @return Object3D* obj itself. If malloc for root failed, returns NULL.
 */
Object3D* Object3D_emplace_triangle(Object3D* obj, 
    Coordinate3D a, Coordinate3D b, Coordinate3D c) 
{
    Triangle3DNode* new_node = malloc(sizeof(Triangle3DNode));
    if(new_node == NULL) {
        // TODO: handle this bad case
    }
    new_node->triangle.a = a;
    new_node->triangle.b = b;
    new_node->triangle.c = c;
    return Object3D_push_triangle_node(obj, new_node);
}

void Object3D_dtor(Object3D* obj) {
    Triangle3DNode *next;
    for(Triangle3DNode* iter = obj->root; iter != NULL; iter = next) {
        next = iter->next;
        free(iter);
    }
    free(obj);
}

Scene3D* Scene3D_create() {
    const int objects_sz = sizeof(Object3D*) * ARRAYLIST_OBJECTS_INITIAL_CAPACITY;
    Scene3D* retval = malloc(sizeof(Scene3D));
    retval->count = 0;
    retval->size = ARRAYLIST_OBJECTS_INITIAL_CAPACITY;
    retval->objects = malloc(objects_sz);
    return retval;
}

void Scene3D_destroy(Scene3D* scene) {
    for(long i = 0; i < scene->count; ++i) {
        // destroy each Object3D
        Object3D_dtor(scene->objects[i]);
    }
    free(scene->objects);
    free(scene);
}

void Scene3D_append(Scene3D* scene, Object3D* object) {
    ++scene->count;
    if(scene->count == scene->size) {
        // need to regrow by doubling.
        // realloc
        Object3D** new = realloc(scene->objects, sizeof(Object3D*) * scene->size * 2);
        if(new == NULL) {
            // TODO: Handle bad case NULL realloc
        }
        scene->size *= 2;
        scene->objects = new; // no need for free because realloc takes care of it for us
    }
    // no more regrow concerns, basic adding.
    scene->objects[scene->count-1] = object;
}


/**
 * A Helper function to append a Triangle3DNode to an Object3D.
 *   Parameters:
 *     object - the object to append to
 *     node   - the node to append
 */
void Object3D_append_object_node(Object3D* object, Triangle3DNode* node) {
  object->count += 1;
  if (object->root == NULL) {
    object->root = node;
    return;
  }
  Triangle3DNode* temp = object->root;
  while (temp->next != NULL) {
     temp = temp->next;
  }
  temp->next = node;
}

/**
 * A Helper function to append a Triangle3D to an Object3D.
 *   Parameters:
 *     object   - the object to append to
 *     triangle - the triangle to append
 */
void Object3D_append_triangle(Object3D* object, Triangle3D triangle) {
  Triangle3DNode* node = calloc(1, sizeof(Triangle3DNode));
  node->triangle = triangle;
  Object3D_append_object_node(object, node);
}

/**
 * Compute the distance between two points.
 *   Parameters:
 *     a / b - Coordinates in 3D space
 *   Return:
 *     The computed distance represented as a double
 */
double Coordinate3D_distance(Coordinate3D a, Coordinate3D b) {
  return sqrt( pow(a.x - b.x, 2.0) + pow(a.y - b.y, 2.0) + pow(a.z - b.z, 2.0));
}

/**
 * Determine the two closest points, and the farthest point, from a given point.
 *   Parameters:
 *     starting - The coordinate to begin distance measuring from
 *     coords   - An array of 3 coordinates 
 *     closest1 / closest2 - The two closest point to starting
 *     Farthest - The farthest point from starting
 */
void Coordinate3D_get_closest_two(
    Coordinate3D starting, Coordinate3D* coords, 
    Coordinate3D* closest1, Coordinate3D* closest2,
    Coordinate3D* farthest) {
  double zero = Coordinate3D_distance(starting, coords[0]);
  double one  = Coordinate3D_distance(starting, coords[1]);
  double two  = Coordinate3D_distance(starting, coords[2]);
  if (zero <= two && one <= two) {
    *closest1 = coords[0];
    *closest2 = coords[1];
    *farthest = coords[2];
  } else if (one <= zero && two <= zero) {
    *closest1 = coords[1];
    *closest2 = coords[2];
    *farthest = coords[0];
  } else if (two <= one && zero <= one){
    *closest1 = coords[0];
    *closest2 = coords[2];
    *farthest = coords[1];
  } else {
    assert(false);
  }
}

#define DOUBLE_MARGIN 0.0001

/**
 * Sort a list of Coordinate3Ds.
 *   Parameters:
 *     array - The array of coordinates to sort
 *     size - the number of elements in the array of coordinates
 */
void Coordinate3D_sort(Coordinate3D* array, int size) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size-1; j++) {
      Coordinate3D ca = array[j];
      Coordinate3D cb = array[j+1];
      if ( (ca.x - cb.x) >= 0.001 ) {
        array[j] = cb; array[j+1] = ca;
      } else if ( (cb.x - ca.x) >= DOUBLE_MARGIN ) {
        continue;
      } else if ( (ca.y - cb.y) >= DOUBLE_MARGIN ) {
        array[j] = cb; array[j+1] = ca;
      } else if ( (cb.y - ca.y) >= DOUBLE_MARGIN ) {
        continue;
      } else if ( (ca.z - cb.z) >= DOUBLE_MARGIN ) {
        array[j] = cb; array[j+1] = ca;
      }
    }
  }
}
#include <stdio.h>

void Object3D_append_quadrilateral(Object3D* o, 
    Coordinate3D a, Coordinate3D b, Coordinate3D c, Coordinate3D d) {

  Coordinate3D starting, closest1, closest2, farthest;
  Coordinate3D co[] = {a, b, c, d};
  Coordinate3D_sort(co, 4);
  a = co[0]; b = co[1]; c = co[2]; d = co[3];
  double max_distance = 0.0;
  double distances[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  int ci = 0;
  int maxi = 0;
  
  // Find the coordinate that is furthest from the other three
  // Store into "starting" and its index into the "distances" array into "ci"
  for (int i = 0; i < 3; i++) {
    for (int j = i+1; j < 4; j++) {
      distances[ci] = Coordinate3D_distance(co[i], co[j]);
      // Handle the case of two points being the "same"
      if (distances[ci] < DOUBLE_MARGIN) {
        Triangle3D single = (Triangle3D){b, c, d}; 
        if (i == 0){
          single = (Triangle3D) {b, c, d};
        } else if (i == 1) {
          single = (Triangle3D) {a, c, d};
        } else if (i == 2) {
          single = (Triangle3D) {a, b, d};
        }
        Object3D_append_triangle(o, single);
        return;
      }
      if (distances[ci] > max_distance) {
        max_distance = distances[ci];
        starting = co[i];
        maxi = ci;
      }
      ci += 1;
    }
  }
 
  // Gather the points other than the starting one
  Coordinate3D tcoords[3] = {b, c, d};
  if (maxi == 3 || maxi == 4) {
    tcoords[0] = a; tcoords[1] = c; tcoords[2] = d;
  } else if (maxi == 5) {
    tcoords[0] = a; tcoords[1] = b; tcoords[2] = d;
  }

  // Put a triangle between starting point, and two closest to it
  Coordinate3D_get_closest_two(starting, tcoords, &closest1, &closest2, &farthest);
  Triangle3D t1 = (Triangle3D) {starting, closest1, closest2};
  Object3D_append_triangle(o, t1); 

  // For the remaining unused point, find two closest, and build second triangle
  tcoords[0] = closest1; tcoords[1] = closest2; tcoords[2] = starting;
  Coordinate3D_get_closest_two(farthest, tcoords, &closest1, &closest2, &starting);
  Triangle3D t2 = (Triangle3D) {farthest, closest1, closest2};
  Object3D_append_triangle(o, t2); 
}
