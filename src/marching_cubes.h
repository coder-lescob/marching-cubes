#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include "mesh.h"

extern const int edgeTable[256];
extern const int triangulationTable[256][16];

/**
 * gives a mesh that is the polygonized version of the `scalar_field_function()`
 */
Mesh marchingcubes_polygonize_region(float (*scalar_field_function)(vec3), vec3 orgine, vec3 size, float cube_size, float iso_level);

/**
 * polygonize a cube
 */
void marchingcubes_polygonize_cell(struct MeshData *data, float (*scalar_field_function)(vec3), vec3 orgine, float cube_size, float iso_level);

/**
 * interpolates the vertex
 */
void interpolate_vertices(float iso_level, vec3 a, vec3 b, float va, float vb, vec3 dest);

#endif