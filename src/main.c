#include <stdio.h>
#include <math.h>

// opengl and glfw
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "shader.h"
#include "mesh.h"
#include "marching_cubes.h"
#include "noise/perlin.h"

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
    // test noise of https://github.com/czinn/perlin
    return pnoise3d(v[0], v[1], v[2], 0.7, 5, 12124);
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

    GLuint marchingcubes_program;
    {
        GLuint marchingcubes_compute = load_and_compile_shader("src/marchingcubes/marchingcubes.glsl", GL_COMPUTE_SHADER);
        marchingcubes_program = create_and_link_program(&marchingcubes_compute, 1);
    }
    
    vec3 null_vec3      = { 0, 0, 0 };
    vec3 marchingRegion = { 25, 25, 25 };

    Mesh mesh = marchingcubes_polygonize(marchingcubes_program, null_vec3, marchingRegion, 1.0f, 0.0f);
    printf("num_triangles: %ld\n", mesh.num_triangles);

    vec3 player_pos = { 0, 0, 0 };
    vec2 player_dir = { 0, 0 };

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glEnable(GL_DEPTH_TEST);

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

        double dx, dy;
        glfwGetCursorPos(window, &dx, &dy);
        glfwSetCursorPos(window, win_width / 2.0, win_height / 2.0);

        vec2 mouse_delta = {dx - win_width / 2.0f, dy - win_height / 2.0f};

        glm_vec2_scale(mouse_delta, 0.05f * TAU * dt, mouse_delta);
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
        glm_translate(model_matrix, (vec3) { -10, -10, -10 } );
        
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