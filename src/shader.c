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
    char *code = preprocess_shader(shader_prog);

    // create a new shader
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, (const GLchar *const *)&code, NULL);
    glCompileShader(shader);

    // since now it has been copied in the shader object it can be freed
    free(shader_prog);
    free(code);

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

bool is_next(char *c, char *patern) {
    for (; *c != 0 && *patern != 0; c++, patern++) {
        if (*c != *patern) {
            return false;
        }
    }
    return *patern == 0;
}

void expects_symbol(char **c, char symbol) {
    if (**c != symbol) {
        fprintf(stderr, "expected %c got %c\n", symbol, **c);
        exit(1);
    }
    (*c)++;
}

void read_until(char **c, char until, char *buf, size_t bufsize) {
    for (size_t i = 0; **c != until && (buf[i] = **c) != 0 && i < bufsize; (*c)++, i++);
}

#define SKIP_SPACE(c) \
    while (*c == ' ') c++;

void push_char(char **s, char c, size_t *capacity) {
    size_t len = strlen(*s);
    if (len + 4 >= *capacity) {
        char *new_s = realloc(*s, *capacity * 2 + 4);
        if (new_s == NULL) {
            fprintf(stderr, "reallocation failed");
            exit(1);
        }
        *s = new_s;
        *capacity = *capacity * 2 + 4;
    }    
    (*s)[len] = c;
    (*s)[len + 1] = 0;
}

void *preprocess_shader(char *content) {
    size_t capacity = 512;
    char *preprocessed = malloc(capacity + 1);
    if (preprocessed == NULL) {
        fprintf(stderr, "allocation failed");
        exit(1);
    }
    memset(preprocessed, 0, capacity + 1);

    size_t line = 0, line_width = 0;
    for (char *c = content; *c != 0; c++) {
        if (line_width == 0) {
            // we are at the start of a line
            if (is_next(c, "#include")) {
                c += 8;
                SKIP_SPACE(c);
                expects_symbol(&c, '"');
                char included_file[512];
                memset(included_file, 0, 512);
                read_until(&c, '"', included_file, sizeof(included_file));
                expects_symbol(&c, '"');

                char *included_content = file_read_all(included_file);
                //printf("included %s\n", included_content);
                for (char *ch = included_content; *ch != 0; ch++) {
                    push_char(&preprocessed, *ch, &capacity);
                }
                free(included_content);
            }
        }

        line_width++;
        if (*c == '\n') {
            line++;
            line_width = 0;
        }
        push_char(&preprocessed, *c, &capacity);
    }

    return preprocessed;
}