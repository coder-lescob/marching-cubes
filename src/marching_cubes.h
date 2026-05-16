#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include "mesh.h"

/**
 * gives a mesh that is the polygonized version of the `scalar_field_function()`
 */
Mesh marchingcubes_polygonize(GLuint marchingcubes_program, vec3 orgine, vec3 size, float cube_size, float iso_level);

#endif