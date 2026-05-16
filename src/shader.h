#ifndef SHADERS_H
#define SHADERS_H

#include <stdbool.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#define CHECK_SHADER_FILE_OPEN(FD)              \
    if (FD == NULL) {                           \
        perror("shader file failed to open");   \
        exit(-1);                               \
    }

// little ansi escape code hacking
#define SET_COLOR_RED     "\x1b[38;5;196m"
#define SET_COLOR_DEFAULT "\x1b[0m"

/**
 * read all the content of a file to string
 * @note the string returned MUST be freed using `free()`
 */
char *file_read_all(char *path);

/**
 * Loads a shader from a file
 */
GLuint load_and_compile_shader(char *path, GLenum shader_type);

/**
 * creates a new program from all shaders given
 */
GLuint create_and_link_program(GLuint *shaders, size_t num_shaders);

/**
 * set the 4x4 matrix `name` to `value` in the GPU program `program`
 */
void set_matrix4x4(GLuint program, char *name, bool transpose, mat4 value);

/**
 * preprocess the shader contents
 */
void *preprocess_shader(char *content);

#endif