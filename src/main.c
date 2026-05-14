#include <stdio.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "shader.h"
#include "mesh.h"

#define CHECK_ERROR_GLFW(ERR_VAL) assert(ERR_VAL == GLFW_TRUE)
#define CHECK_OBJ_ERROR(OBJ_PTR)                \
    if (OBJ_PTR == NULL) {                      \
        perror(#OBJ_PTR " creation failed");    \
        exit(1);                                \
    }

#define ARRAY_LEN(ARRAY, TYPE) (sizeof(ARRAY) / sizeof(TYPE))

/**
 * inits the window and glfw and glew contexts
 */
void init(GLFWwindow **window) {
    CHECK_ERROR_GLFW(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    *window = glfwCreateWindow(480, 480, "Marching Cubes", glfwGetPrimaryMonitor(), NULL);
    CHECK_OBJ_ERROR(*window);
    glfwMakeContextCurrent(*window);

    glewExperimental = GL_TRUE;  // Enable experimental features for modern OpenGL
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(1);
    }
}

int main(void) {
    GLFWwindow *window;
    int win_width, win_height;

    init(&window);

    GLuint program;
    {
        GLuint vertex_shader = load_and_compile_shader("src/shaders/vert3D.glsl", GL_VERTEX_SHADER);
        GLuint fragment_shader = load_and_compile_shader("src/shaders/frag3D.glsl", GL_FRAGMENT_SHADER);
        GLuint shaders[2] = { vertex_shader, fragment_shader };
        program = create_and_link_program(shaders, 2);
    }

    // vertex data from https://learnopengl.com/Getting-started/Hello-Triangle, for testing
    vec3 vertices[] = {
        {  0.5f,  0.5f, 0.0f },  // top right
        {  0.5f, -0.5f, 0.0f },  // bottom right
        { -0.5f, -0.5f, 0.0f },  // bottom left
        { -0.5f,  0.5f, 0.0f },  // top left 
    };

    vec2 uvs[] = {
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 1.0f },
    };

    GLuint indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    Mesh mesh = new_mesh(GL_DYNAMIC_DRAW, vertices, uvs, ARRAY_LEN(vertices, vec3), indices, ARRAY_LEN(indices, GLuint));

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &win_width, &win_height);

        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        {
            render_mesh(&mesh);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    glDeleteProgram(program);
    free_mesh(&mesh);

    glfwDestroyWindow(window);
}