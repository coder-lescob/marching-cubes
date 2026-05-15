#ifndef MESH_H
#define MESH_H

#include <stdlib.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

struct MeshData {
    vec3   *vertices;
    vec2   *uvs;
    GLuint *triangles;
    size_t num_vertices, num_triangles;

    float *cached_vert_uv_data;
    bool cached;
};

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
Mesh new_mesh(GLenum mesh_kind, struct MeshData *mesh_data);

/**
 * frees the mesh by destroying all opengl objs
 */
void free_mesh(Mesh *mesh);

/**
 * create a new mesh data
 * @note this function does cache the compined vertices and uvs for speed
 */
struct MeshData new_mesh_data(
    vec3 *vertices, vec2 *uvs, GLuint *triangles,
    size_t num_vertices, size_t num_triangles
);

/**
 * free the mesh data
 */
void free_mesh_data(struct MeshData *mesh_data);

/**
 * updates the mesh
 */
void update_mesh(Mesh *mesh, struct MeshData *mesh_data);

/**
 * renders the mesh using program currently in use
 */
void render_mesh(Mesh *mesh);

/**
 * pushes the triangle p1p2p3 in the meshdata `data`
 */
void push_triangle(struct MeshData *data, vec3 p1, vec3 p2, vec3 p3);

#endif