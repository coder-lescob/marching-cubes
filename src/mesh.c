#include "mesh.h"

#include <stdio.h>
#include <string.h>

#define IS_AUTHORISED_MESH_KIND(TYPE) \
    ((TYPE) == GL_STATIC_DRAW         \
  || (TYPE) == GL_DYNAMIC_DRAW        \
  || (TYPE) == GL_STREAM_DRAW)

#define IS_MESH_FREED(MESH_PTR)                                     \
    (MESH_PTR)->num_vertices == 0 || (MESH_PTR)->num_triangles == 0     \
 || (MESH_PTR)->VAO == 0 || (MESH_PTR)->VBO == 0 || (MESH_PTR)->EBO == 0

float *merge_vertex_and_uvs(vec3 *vertices, vec2 *uvs, size_t num_vertices) {
    /**
     * format:
     * vertexA, uvA,
     * vertexB, uvB,
     * ...
     */
    size_t stride = sizeof(vec3) + sizeof(vec2);
    float *merged_data = calloc(num_vertices, stride);

    // merge data
    size_t j = 0;
    for (size_t i = 0; i < num_vertices * stride; i += stride) {
        memcpy((char *)merged_data + i + 0, &vertices[j], sizeof(vec3));
        memcpy((char *)merged_data + i + sizeof(vec3), &uvs[j], sizeof(vec2));
        j++;
    }

    return merged_data;
}

Mesh new_mesh(GLenum mesh_kind, vec3 *vertices, vec2 *uvs, size_t num_vertices, 
    GLuint *triangles, size_t num_triangles) {

    if (!IS_AUTHORISED_MESH_KIND(mesh_kind)) {
        fprintf(stderr, "Unauthorised mesh kind %d", mesh_kind);
        exit(1);
    }

    // create VAO, VBO and EBO
    GLuint VAO, VBO, EBO;
    glCreateVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    {
        /**
         * format:
         * vertexA, uvA,
         * vertexB, uvB,
         * ...
         */
        size_t stride = sizeof(vec3) + sizeof(vec2);
        {
            // merge the vertices and uvs together
            float *data = merge_vertex_and_uvs(vertices, uvs, num_vertices);

            // send raw data to the GPU
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, num_vertices * stride, data, mesh_kind);

            // free the data buffer
            free(data);
        }

        // send the raw triangles indices to the GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_triangles * sizeof(int), triangles, mesh_kind);
        
        // The GPU is extreamly dumb, so we must explain the format to it
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)sizeof(vec3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }
    glBindVertexArray(0);

    return (Mesh) {
        .mesh_kind = mesh_kind,
        .VAO = VAO,
        .VBO = VBO,
        .EBO = EBO,
        .num_vertices = num_vertices,
        .num_triangles = num_triangles,
    };
}

void free_mesh(Mesh *mesh) {
    if (IS_MESH_FREED(mesh)) return;

    // delete all buffers
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);

    // delete the vertex array
    glDeleteVertexArrays(1, &mesh->VAO);

    // freed
    mesh->mesh_kind = 0;
    mesh->num_triangles = 0;
    mesh->num_vertices = 0;
}

void update_vertices(Mesh *mesh, GLintptr offset, vec3 *vertices, 
    vec2 *uvs, size_t num_vertices) {

    if (IS_MESH_FREED(mesh)) return;

    glBindVertexArray(mesh->VAO);
    {
        size_t stride = sizeof(vec3) + sizeof(vec2);
        float *data = merge_vertex_and_uvs(vertices, uvs, num_vertices);

        // update the buffer
        if (mesh->num_vertices >= num_vertices) {
            glBufferSubData(GL_ARRAY_BUFFER, offset, num_vertices * stride, data);
        }
        else {
            // else we must reallocate the entier buffer
            glBufferData(GL_ARRAY_BUFFER, num_vertices * stride, data, mesh->mesh_kind);
        }

        free(data);
    }
    glBindVertexArray(0);
}

void update_triangles(Mesh *mesh, GLintptr offset, 
    GLuint *triangles, size_t num_triangles) {
        
    if (IS_MESH_FREED(mesh)) return;
    
    glBindVertexArray(mesh->VAO);
    {
        if (mesh->num_triangles >= num_triangles) {
            // update the buffer
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, num_triangles * sizeof(int), triangles);
        }
        else {
            // we must reallocate
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_triangles * sizeof(int), triangles, mesh->mesh_kind);
        }
    }
    glBindVertexArray(0);
}

void render_mesh(Mesh *mesh) {
    if (IS_MESH_FREED(mesh)) return;

    glBindVertexArray(mesh->VAO);
    {
        // draw the mesh as... believe me or not: triangles
        glDrawElements(GL_TRIANGLES, mesh->num_triangles, GL_UNSIGNED_INT, (void *)0);
    }
    glBindVertexArray(0);
}