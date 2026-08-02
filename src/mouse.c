#include "mouse.h"
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

/**
 * the callback that is used to move the mouse
 */
void mouse_move_callback(GLFWwindow *window, double xpos, double ypos) {

    // get the mouse data from the user pointer
    MouseMove* mouse_move = glfwGetWindowUserPointer(window);

    if (mouse_move == NULL) {
        perror("no mouse move given through window user pointer.");
        exit(1);
    }

    // figure out how much the mouse moved
    mouse_move->dx = xpos - mouse_move->xpos;
    mouse_move->dy = ypos - mouse_move->ypos;

    // record the position
    mouse_move->xpos = xpos;
    mouse_move->ypos = ypos;
}