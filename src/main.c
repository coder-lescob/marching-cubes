#include <stdio.h>
#include <math.h>

// opengl and glfw
#include <epoxy/gl.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "shader.h"
#include "mesh.h"
#include "marching_cubes.h"
#include "noise/perlin.h"

#include "player.h"

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
        glfwTerminate();                        \
        exit(1);                                \
    }

#define ARRAY_LEN(ARRAY, TYPE) (sizeof(ARRAY) / sizeof(TYPE))

/**
 * inits the window and glfw context
 */
void init(GLFWwindow **window) {
    CHECK_ERROR_GLFW(glfwInit());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    *window = glfwCreateWindow(480, 480, "Marching Cubes", glfwGetPrimaryMonitor(), NULL);
    CHECK_OBJ_ERROR(*window);
    glfwMakeContextCurrent(*window);
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
    vec3 marchingRegion = { 100, 100, 100 };

    Mesh mesh = marchingcubes_polygonize(marchingcubes_program, null_vec3, marchingRegion, 1.0f, 0.0f);
    printf("num_triangles: %ld\n", mesh.num_triangles);
    
    GLuint postprocessing_program;
    {
        GLuint vertex_shader = load_and_compile_shader("src/shaders/postprocess.vert", GL_VERTEX_SHADER);
        GLuint fragment_shader = load_and_compile_shader("src/shaders/postprocess.frag", GL_FRAGMENT_SHADER);
        GLuint shaders[2] = { vertex_shader, fragment_shader };
        postprocessing_program = create_and_link_program(shaders, 2);
    }

    struct Player player = {
        .pos = { 0, 0, 0 },
        .dir = { 0, 0 },
    };

    glm_mat4_identity(player.view_matrix);
    glfwSetCursorPos(window, win_width / 2.0, win_height / 2.0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  

    double last_mesured_time = glfwGetTime(), dt = 0;
    glfwGetWindowSize(window, &win_width, &win_height);

    vec3 quad_vertices[] = {
        { -1.0f, -1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f },
        { -1.0f,  1.0f, 0.0f },
    };

    vec2 quad_uvs[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
    };

    GLuint quad_tris[] = {
        0, 1, 2,
        0, 2, 3,
    };

    struct MeshData quad_data = new_mesh_data(quad_vertices, quad_uvs, quad_tris, 4, 2);
    Mesh quad_mesh = new_mesh(GL_STATIC_DRAW, &quad_data);

    GLuint fbo, target_texture, depth_texture;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &target_texture);
    glGenTextures(1, &depth_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        printf("%dx%d\n", win_width, win_height);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, win_width, win_height);

        // frame buffer init
        // texture init
        glBindTexture(GL_TEXTURE_2D, target_texture);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_width, win_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // attach the texture to the frame buffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target_texture, 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, depth_texture);
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, win_width, win_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // attach the texture to the frame buffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        
        GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);

        // check for completion
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fprintf(stderr, "frame buffer initialitialization failed\n");
            exit(1);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window)) {
        dt = glfwGetTime() - last_mesured_time;
        last_mesured_time = glfwGetTime();

        glfwGetWindowSize(window, &win_width, &win_height);

        update_player(&player, window, dt);

        mat4 projection_matrix;
        glm_perspective(glm_rad(60), win_width / (float)win_height, 0.1f, 100.0f, projection_matrix);

        mat4 model_matrix;

        glm_mat4_identity(model_matrix);
        glm_translate(model_matrix, (vec3) { -50, -50, -50 } );

        glViewport(0, 0, win_width, win_height);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            glUseProgram(program);
            {
                set_matrix4x4(program, "projection_matrix", false, projection_matrix);
                set_matrix4x4(program, "view_matrix", false, player.view_matrix);
                set_matrix4x4(program, "model_matrix", false, model_matrix);
                render_mesh(&mesh);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(postprocessing_program);
        {
            set_matrix4x4(postprocessing_program, "projection_matrix", false, projection_matrix);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, target_texture);
            glUniform1i(glGetUniformLocation(postprocessing_program, "screenTexture"), 0);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depth_texture);
            glUniform1i(glGetUniformLocation(postprocessing_program, "depthTexture"), 1);

            render_mesh(&quad_mesh);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    glDeleteProgram(program);
    free_mesh(&mesh);
    glDeleteFramebuffers(1, &fbo);

    glfwDestroyWindow(window);
}