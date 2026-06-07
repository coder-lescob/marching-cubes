#include "player.h"

#include <cglm/cglm.h>

/**
 * I really needs that much digits (51 digits)
 * https://en.wikipedia.org/wiki/Tau_(mathematics)
 */
#define TAU 6.28318530717958647692528676655900576839433879875021
#define PI 3.141592653589793238462643383279502884197169399375105

#define CLAMP(X, A, B)          \
    fmax((A), fmin((B), (X)))

/**
 * update the entier player
 * @param player a pointer to the player
 * @param window a pointer to the window
 */
void update_player(struct Player *player, GLFWwindow *window, double dt) {
    update_dir(player, window, dt);
    update_pos(player, window, dt);
    update_view_matrix(player);
}

/**
 * update the position of the player
 * @param player a pointer to the player
 * @param window a pointer to the window
 */
void update_pos(struct Player *player, GLFWwindow *window, double dt) {
    vec3 player_input = { 0, 0, 0 };

    // grab the input
    player_input[2] = glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S);
    player_input[0] = glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A);

    // rotate around the basis vectors to move relativaly to the player
    glm_vec3_rotate(player_input, player->dir[1], (vec3) { 1, 0, 0 });
    glm_vec3_rotate(player_input, player->dir[0], (vec3) { 0, 1, 0 });
    
    // update the position
    for (int axis = 0; axis < 3; axis++) {
        player->pos[axis] += 5 * player_input[axis] * dt;
    }
}

/**
 * update the direction of the player
 * @param player a pointer to the player
 * @param window a pointer to the window
 */
void update_dir(struct Player *player, GLFWwindow *window, double dt) {
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);

    double dx, dy;
    glfwGetCursorPos(window, &dx, &dy);
    glfwSetCursorPos(window, win_width / 2.0, win_height / 2.0);

    // compute the delta from the center of the screen
    vec2 mouse_delta = {
        dx - win_width  / 2.0f, 
        dy - win_height / 2.0f,
    };

    // scale it and rotate
    glm_vec2_scale(mouse_delta, 0.05f * TAU * dt, mouse_delta);
    glm_vec2_add(player->dir, mouse_delta, player->dir);

    player->dir[1] = CLAMP(player->dir[1], -PI / 2, PI / 2);
}

/**
 * updates the view matrix.
 * @param player a pointer to the player
 */
void update_view_matrix(struct Player *player) {
    // set the view matrix to the identity (no transformation)
    glm_mat4_identity(player->view_matrix);

    // rotate the matrix
    glm_rotate(player->view_matrix, player->dir[1], (vec3){ 1, 0, 0 }); // rotate around î
    glm_rotate(player->view_matrix, player->dir[0], (vec3){ 0, 1, 0 }); // rotate around ĵ

    // translate the view camera by the opposite of the player position to give the movement effect
    glm_translate(player->view_matrix, (vec3) { -player->pos[0], -player->pos[1], player->pos[2] });
}