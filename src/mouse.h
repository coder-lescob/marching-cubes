#ifndef MOUSE_H
#define MOUSE_H

#include <GLFW/glfw3.h>

typedef struct {
    double xpos, ypos;
    double dx, dy;
} MouseMove;

/**
 * the callback that is used to move the mouse
 */
void mouse_move_callback(GLFWwindow *window, double xpos, double ypos);

#endif