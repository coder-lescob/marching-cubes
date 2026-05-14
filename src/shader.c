#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

char *file_read_all(char *path) {
    // open the file
    FILE *fd = fopen(path, "r");
    CHECK_SHADER_FILE_OPEN(fd);

    // get the size of the file
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    
    char *file_contents = malloc(size + 1);
    if (file_contents == NULL) {
        perror("unable to allocate read buffer");
        exit(1);
    }
    memset(file_contents, 0, size + 1);

    // read all the contents
    fread(file_contents, size, size, fd);

    return file_contents;
}

GLuint load_and_compile_shader(char *path, GLenum shader_type) {
    // read all the content of the shader file
    char *shader_prog = file_read_all(path);

    // create a new shader
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const GLchar *const *)&shader_prog, NULL);
    glCompileShader(shader);

    // since now it has been copied in the shader object it can be freed
    free(shader_prog);

    int success;
    char infolog[512]; // I've no clue why 512
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        // get the info log and exit
        glGetShaderInfoLog(shader, 512, NULL, infolog);
        glDeleteShader(shader);

        // print the error in red
        fprintf(stderr, SET_COLOR_RED "%s\n" SET_COLOR_DEFAULT, infolog);
        exit(1);
    }

    return shader;
}

GLuint create_and_link_program(GLuint *shaders, size_t num_shaders) {
    // create a program
    GLuint prog = glCreateProgram();

    // attach all shaders
    for (size_t i = 0; i < num_shaders; i++) {
        glAttachShader(prog, shaders[i]);
    }

    // link it
    glLinkProgram(prog);

    // once linked destroy all shaders because it does not need them anymore
    for (size_t i = 0; i < num_shaders; i++) {
        glDeleteShader(shaders[i]);
    }

    // verify link errors
    int success;
    char infolog[512]; // I've no clue why 512
    glGetProgramiv(prog, GL_LINK_STATUS, &success);

    if (!success) {
        // get the info log and exit
        glGetProgramInfoLog(prog, 512, NULL, infolog);
        glDeleteProgram(prog);

        // print the error in red
        fprintf(stderr, SET_COLOR_RED "%s\n" SET_COLOR_DEFAULT, infolog);
        exit(1);
    }

    return prog;
}

void set_matrix4x4(GLuint program, char *name, bool transpose, mat4 value) {
    // first get the location of the uniform
    GLint location = glGetUniformLocation(program, name);

    if (location == -1) {
        // then the unform does not exist
        fprintf(stderr, "Unable to locate uniform matrix4x4 %s\n", name);
        exit(1);
    }

    // uniform location found so now we can send the value to the GPU
    glUniformMatrix4fv(location, 1, transpose, (float *) { *value } );
}