#include <stdio.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "shader.h"
#include "mesh.h"
#include "marching_cubes.h"

typedef double vec2d[2];

/**
 * I really needs that much digits (51 digits)
 * https://en.wikipedia.org/wiki/Tau_(mathematics)
 */
#define TAU 6.28318530717958647692528676655900576839433879875021
#define PI 3.141592653589793238462643383279502884197169399375105 

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

float field_function(vec3 v) {
    vec3 center = { 5, 5, 5 };
    glm_vec3_sub(v, center, v);
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] - 25;
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

    vec3 null_vec3 = { 0, 0, 0 };
    vec3 size      = { 25, 25, 25 };
    Mesh mesh = marchingcubes_polygonize_region(field_function, null_vec3, size, 1.0f, 0);

    vec3 player_pos = { 0, 0, 0 };
    vec2 player_dir = { 0, 0 };
    vec2d mouse_last_pos;
    glfwGetCursorPos(window, &mouse_last_pos[0], &mouse_last_pos[1]);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glEnable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    double last_mesured_time = glfwGetTime(), dt = 0;

    while (!glfwWindowShouldClose(window)) {
        dt = glfwGetTime() - last_mesured_time;
        last_mesured_time = glfwGetTime();

        glfwGetWindowSize(window, &win_width, &win_height);

        vec3 player_input = { 0, 0, 0 };

        if (glfwGetKey(window, GLFW_KEY_W)) {
            player_input[2] += 1;
        }
        
        if (glfwGetKey(window, GLFW_KEY_S)) {
            player_input[2] -= 1;
        }

        if (glfwGetKey(window, GLFW_KEY_D)) {
            player_input[0] += 1;
        }

        if (glfwGetKey(window, GLFW_KEY_A)) {
            player_input[0] -= 1;
        }

        vec2d mouse_pos;
        glfwGetCursorPos(window, &mouse_pos[0], &mouse_pos[1]);

        vec2 mouse_delta = {0, 0};
        mouse_delta[0] = (float)(mouse_pos[0] - mouse_last_pos[0]) * 0.05f * TAU * dt;
        mouse_delta[1] = (float)(mouse_pos[1] - mouse_last_pos[1]) * 0.05f * TAU * dt;
        mouse_last_pos[0] = mouse_pos[0];
        mouse_last_pos[1] = mouse_pos[1];

        glm_vec2_add(player_dir, mouse_delta, player_dir);

        if (player_dir[1] < -PI / 2) {
            player_dir[1] = -PI / 2;
        }
        if (player_dir[1] > PI / 2) {
            player_dir[1] = PI / 2;
        }
        
        glm_vec3_rotate(player_input, player_dir[1], (vec3) { 1, 0, 0 });
        glm_vec3_rotate(player_input, player_dir[0], (vec3) { 0, 1, 0 });
        
        for (int axis = 0; axis < 3; axis++) {
            player_pos[axis] += 5 * player_input[axis] * dt;
        }

        mat4 projection_matrix;
        glm_perspective(glm_rad(60), win_width / (float)win_height, 0.1f, 100.0f, projection_matrix);

        mat4 model_matrix, view_matrix;

        glm_mat4_identity(model_matrix);
        glm_translate(model_matrix, (vec3) { 0, 0, -5 } );
        
        glm_mat4_identity(view_matrix);
        glm_rotate(view_matrix, player_dir[1], (vec3){1, 0, 0});
        glm_rotate(view_matrix, player_dir[0], (vec3){0, 1, 0});
        glm_translate(view_matrix, (vec3) { -player_pos[0], -player_pos[1], player_pos[2] });

        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        {
            set_matrix4x4(program, "projection_matrix", false, projection_matrix);
            set_matrix4x4(program, "view_matrix", false, view_matrix);
            set_matrix4x4(program, "model_matrix", false, model_matrix);
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