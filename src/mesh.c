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

#define NO_DANGLE_FREE(PTR) \
    if (PTR != NULL) {      \
        free(PTR);          \
        PTR = NULL;         \
    }

#define CHECK_ALLOC(PTR)            \
    if (PTR == NULL) {              \
        perror("allocation failed");\
        exit(1);                    \
    }

float *merge_vertex_and_uvs(vec3 *vertices, vec2 *uvs, size_t num_vertices) {
    /**
     * format:
     * vertexA, uvA,
     * vertexB, uvB,
     * ...
     */
    size_t stride = sizeof(vec3) + sizeof(vec2);
    float *merged_data = calloc(num_vertices, stride);
    CHECK_ALLOC(merged_data);

    // merge data
    size_t j = 0;
    for (size_t i = 0; i < num_vertices * stride; i += stride) {
        memcpy((char *)merged_data + i + 0, &vertices[j], sizeof(vec3));
        memcpy((char *)merged_data + i + sizeof(vec3), &uvs[j], sizeof(vec2));
        j++;
    }

    return merged_data;
}

Mesh new_mesh(GLenum mesh_kind, struct MeshData *mesh_data) {

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
            float *data;

            if (mesh_data->cached) {
                data = mesh_data->cached_vert_uv_data;
            }
            else {
                // merge the vertices and uvs together
                data = merge_vertex_and_uvs(mesh_data->vertices, mesh_data->uvs, mesh_data->num_vertices);
            }

            if (data == NULL) {
                perror("mesh allocation failed");
                exit(1);
            }

            // send raw data to the GPU
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, mesh_data->num_vertices * stride, data, mesh_kind);
            
            // cache data
            if (!mesh_data->cached) {
                mesh_data->cached_vert_uv_data = data;
                mesh_data->cached = true;
            }
        }

        // send the raw triangles indices to the GPU
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data->num_triangles * sizeof(int), mesh_data->triangles, mesh_kind);
        
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
        .num_vertices = mesh_data->num_vertices,
        .num_triangles = mesh_data->num_triangles,
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

struct MeshData new_mesh_data(
    vec3 *vertices, vec2 *uvs, GLuint *triangles,
    size_t num_vertices, size_t num_triangles
) {
    if (vertices == NULL || uvs == NULL || triangles == NULL) {
        perror("NULL buffer found during MeshData creation");
        exit(1);
    }
    
    // allocate the buffers
    vec3 *mesh_vertices = calloc(num_vertices, sizeof(vec3));
    CHECK_ALLOC(mesh_vertices);
    vec2 *mesh_uvs = calloc(num_vertices, sizeof(vec2));
    CHECK_ALLOC(mesh_uvs);
    GLuint *mesh_triangles = calloc(num_triangles, sizeof(GLuint));
    CHECK_ALLOC(mesh_triangles);

    // copy the data
    memcpy(mesh_vertices, vertices, num_vertices * sizeof(vec3));
    memcpy(mesh_uvs, uvs, num_vertices * sizeof(vec2));
    memcpy(mesh_triangles, triangles, num_triangles * sizeof(GLuint));

    // cache the data
    float *cached_data = merge_vertex_and_uvs(mesh_vertices, mesh_uvs, num_vertices);

    return (struct MeshData) {
        .vertices  = mesh_vertices,
        .uvs       = mesh_uvs,
        .triangles = mesh_triangles,
        
        .num_vertices  = num_vertices,
        .num_triangles = num_triangles,

        .cached_vert_uv_data = cached_data,
        .cached              = true
    };
}

void free_mesh_data(struct MeshData *mesh_data) {
    if (mesh_data == NULL) return;

    NO_DANGLE_FREE(mesh_data->vertices);
    NO_DANGLE_FREE(mesh_data->uvs);
    NO_DANGLE_FREE(mesh_data->triangles);
    NO_DANGLE_FREE(mesh_data->cached_vert_uv_data);

    mesh_data->cached = false;
    mesh_data->num_vertices  = 0;
    mesh_data->num_triangles = 0;
}

void update_mesh(Mesh *mesh, struct MeshData *mesh_data) {
    if (IS_MESH_FREED(mesh)) return;
    
    glBindVertexArray(mesh->VAO);
    {
        float *data;
        size_t stride = sizeof(vec3) + sizeof(vec2);
        if (mesh_data->cached) {
            data = mesh_data->cached_vert_uv_data;
        }
        else {
            data = merge_vertex_and_uvs(mesh_data->vertices, mesh_data->uvs, mesh_data->num_vertices);
        }

        // update the buffer
        if (mesh->num_vertices == mesh_data->num_vertices) {
            // this is faster than reallocating
            glBufferSubData(GL_ARRAY_BUFFER, 0, mesh_data->num_vertices * stride, data);
        }
        else {
            // else we must reallocate the entier buffer
            glBufferData(GL_ARRAY_BUFFER, mesh_data->num_vertices * stride, data, mesh->mesh_kind);
        }

        // update the buffer
        if (mesh->num_triangles == mesh_data->num_triangles) {
            // faster than reallocating
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh_data->num_triangles * sizeof(int), mesh_data->triangles);
        }
        else {
            // we must reallocate
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data->num_triangles * sizeof(int), mesh_data->triangles, mesh->mesh_kind);
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

void push_triangle(struct MeshData *data, vec3 p1, vec3 p2, vec3 p3) {
    vec3 *new_vertices = realloc(data->vertices, (data->num_vertices + 3) * sizeof(vec3));
    vec2 *new_uvs = realloc(data->uvs, (data->num_vertices + 3) * sizeof(vec2));
    GLuint *new_triangles = realloc(data->triangles, (data->num_triangles + 3) * sizeof(GLuint));

    if (new_vertices == NULL || new_uvs == NULL || new_triangles == NULL) {
        perror("triangle push failed: reallocation failed");
        exit(1);
    }

    // vertices
    memcpy(new_vertices[data->num_vertices + 0], p1, sizeof(vec3));
    memcpy(new_vertices[data->num_vertices + 1], p2, sizeof(vec3));
    memcpy(new_vertices[data->num_vertices + 2], p3, sizeof(vec3));

    // uvs, for now allays 0, 0
    vec2 null_vec2 = { 0, 0 };
    memcpy(new_uvs[data->num_vertices + 0], null_vec2, sizeof(vec2));
    memcpy(new_uvs[data->num_vertices + 1], null_vec2, sizeof(vec2));
    memcpy(new_uvs[data->num_vertices + 2], null_vec2, sizeof(vec2));

    // triangles
    new_triangles[data->num_triangles + 0] = data->num_vertices + 0;
    new_triangles[data->num_triangles + 1] = data->num_vertices + 1;
    new_triangles[data->num_triangles + 2] = data->num_vertices + 2;

    data->vertices = new_vertices;
    data->uvs      = new_uvs;
    data->triangles = new_triangles;
    data->num_vertices += 3;
    data->num_triangles += 3;
}