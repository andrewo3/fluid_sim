#include "mouse.hpp"
#include <SDL3/SDL.h>

Mouse::Mouse(bool autom, SDL_Window* win, int gw, int gh) {
    automated = autom;
    window = win;
    SDL_GetWindowSize(window,window_size,window_size+1);
    grid_w = gw;
    grid_h = gh;
}

void Mouse::getPos(int* wr) {
    if (!automated) {
        float mouse_pos[2];
        SDL_GetMouseState(mouse_pos,mouse_pos+1);

        wr[0] = (int)(mouse_pos[0] / window_size[0] * grid_w);
        wr[1] = (int)(grid_h - mouse_pos[1] / window_size[1] * grid_h);
    }
}

void Mouse::getVel(float* wr) {
    if (!automated) {
        float mouse_vel[2];
        SDL_GetRelativeMouseState(mouse_vel,mouse_vel+1);

        wr[0] = mouse_vel[0] / window_size[0] * grid_w;
        wr[1] = -(mouse_vel[1] / window_size[1] * grid_h);
    }
}

void Mouse::getButtons(int* wr) {
    if (!automated) {
        SDL_MouseButtonFlags buttons = SDL_GetMouseState(NULL, NULL);
        wr[0] = buttons & SDL_BUTTON_LMASK;
        wr[1] = buttons & SDL_BUTTON_MMASK;
        wr[2] = buttons & SDL_BUTTON_RMASK;
    }
}