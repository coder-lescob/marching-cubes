#ifndef PLAYER_H
#define PLAYER_H

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

struct Player {
    // the position and direction of the player
    vec3 pos;
    vec2 dir;

    // the point of view of the player
    mat4 view_matrix;
};

/**
 * update the entier player
 * @param player a pointer to the player
 * @param window a pointer to the window
 * @param dt the amount of time between two frames
 */
void update_player(struct Player *player, GLFWwindow *window, double dt);

/**
 * update the position of the player
 * @param player a pointer to the player
 * @param window a pointer to the window
 * @param dt the amount of time between two frames
 */
void update_pos(struct Player *player, GLFWwindow *window, double dt);

/**
 * update the direction of the player
 * @param player a pointer to the player
 * @param window a pointer to the window
 * @param dt the amount of time between two frames
 */
void update_dir(struct Player *player, GLFWwindow *window, double dt);

/**
 * updates the view matrix.
 * @param player a pointer to the player
 */
void update_view_matrix(struct Player *player);

#endif