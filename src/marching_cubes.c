#include "marching_cubes.h"

#include <string.h>

Mesh marchingcubes_polygonize(GLuint marchingcubes_program, vec3 orgine, vec3 marchingRegion, float cube_size, float iso_level) {

    size_t num_cubes  = marchingRegion[0] * marchingRegion[1] * marchingRegion[2];
    size_t max_num_vertices  = num_cubes * 15;
    size_t max_num_triangles = num_cubes * 5;

    // create the buffer to hold the mesh back to the CPU
    GLuint verts;
    glGenBuffers(1, &verts);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, verts);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_num_vertices * sizeof(vec3), NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, verts);

    GLuint tris;
    glGenBuffers(1, &tris);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tris);
    glBufferData(GL_SHADER_STORAGE_BUFFER, max_num_triangles * TRIANGLE_SIZE, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tris);

    // initialize the counters to 0
    GLuint counters;
    GLuint counters_init_val[2] = { 0, 0 };
    glGenBuffers(1, &counters);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counters);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(counters_init_val), counters_init_val, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, counters);

    glUseProgram(marchingcubes_program);
    {
        glUniform3f(glGetUniformLocation(marchingcubes_program, "_Origine"), orgine[0], orgine[1], orgine[2]);
        glUniform1f(glGetUniformLocation(marchingcubes_program, "_CubeSize"), cube_size);
        glUniform1f(glGetUniformLocation(marchingcubes_program, "_IsoLevel"), iso_level);
        glUniform3f(glGetUniformLocation(marchingcubes_program, "_RegionSize"), marchingRegion[0], marchingRegion[1], marchingRegion[2]);
        glDispatchCompute(marchingRegion[0] / 8, marchingRegion[1] / 8, marchingRegion[2] / 8);

        // wait for data to come back
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // read the counts back
    GLuint num_vertices, num_triangles;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, counters);
    {
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &num_vertices);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), sizeof(GLuint), &num_triangles);
    }

    // read the buffers
    // This dumb GPU has padding to get to 16 bytes by elements
    vec4 *vertices = calloc(num_vertices, sizeof(vec4));
    vec2 *uvs      = calloc(num_vertices, sizeof(vec2)); // filled with 0s
    struct Tri *triangles = calloc(num_triangles, sizeof(struct Tri));
    
    // read the buffers back
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, verts);
    {
        // read the buffer
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num_vertices * sizeof(vec4), vertices);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tris);
    {
        // read the buffer
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num_triangles * sizeof(struct Tri), triangles);
    }

    // create the mesh
    struct MeshData mesh_data = new_mesh_data(pack_data(vertices, num_vertices), uvs, pack_tris(triangles, num_triangles), num_vertices, num_triangles);
    return new_mesh(GL_DYNAMIC_DRAW, &mesh_data);
}