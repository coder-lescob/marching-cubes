#include <stdio.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

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

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &win_width, &win_height);

        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);

        /**
         * TODO: add actual rendering
         */

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    glfwDestroyWindow(window);
}