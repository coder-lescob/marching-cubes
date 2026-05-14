#ifndef MESH_H
#define MESH_H

#include <stdlib.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

typedef struct Mesh {
    // dynamic_draw / static_draw / stream_draw
    GLenum mesh_kind;

    // VertexArrayObject, VertexBufferObject and ElementBufferObject
    GLuint VAO, VBO, EBO;
    size_t num_vertices, num_triangles;
} Mesh;

/**
 * creates a new mesh to have this data.
 */
Mesh new_mesh(GLenum mesh_kind, vec3 *vertices, vec2 *uvs, size_t num_vertices, 
    GLuint *triangles, size_t num_triangles);

/**
 * frees the mesh by destroying all opengl objs
 */
void free_mesh(Mesh *mesh);

/**
 * updates the vertices of the mesh
 */
void update_vertices(Mesh *mesh, GLintptr offset, vec3 *vertices, 
    vec2 *uvs, size_t num_vertices);

/**
 * updates the triangles of the mesh
 */
void update_triangles(Mesh *mesh, GLintptr offset, 
    GLuint *triangles, size_t num_triangles);

/**
 * renders the mesh using program currently in use
 */
void render_mesh(Mesh *mesh);

#endif