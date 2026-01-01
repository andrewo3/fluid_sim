#include "mouse.hpp"
#include <SDL3/SDL.h>
#include <cstdlib>

Mouse::Mouse(bool autom, SDL_Window* win, int gw, int gh) {
    automated = autom;
    window = win;
    SDL_GetWindowSize(window,window_size,window_size+1);
    grid_w = gw;
    grid_h = gh;
}

float Mouse::interp(int start, int end, float t) {
    return (start + t * ( end - start));
}

void Mouse::update() {
    if (!automated) {
        SDL_GetMouseState(mouse_pos,mouse_pos+1);
        SDL_GetRelativeMouseState(mouse_vel,mouse_vel+1);
        SDL_MouseButtonFlags buttons = SDL_GetMouseState(NULL, NULL);
        mouse_buttons[0] = buttons & SDL_BUTTON_LMASK;
        mouse_buttons[1] = buttons & SDL_BUTTON_MMASK;
        mouse_buttons[2] = buttons & SDL_BUTTON_RMASK;
    } else {
        long time = SDL_GetTicks();
        if (!moving) {
            if (time - last_time > duration) {
                start_pos[0] = rand() % window_size[0];
                start_pos[1] = rand() % window_size[1];

                end_pos[0] = rand() % window_size[0];
                end_pos[1] = rand() % window_size[1];

                action = rand() % 4;
                action %= 3;

                mouse_pos[0] = interp(start_pos[0], end_pos[0], 0);
                mouse_pos[1] = interp(start_pos[1], end_pos[1], 0);

                last_mouse_pos[0] = mouse_pos[0];
                last_mouse_pos[1] = mouse_pos[1];

                duration = LENGTH_MIN + (rand() % (LENGTH_MAX - LENGTH_MIN));
                last_time = time;
                moving = true;
            }
        } else {
            int t = time - last_time;
            mouse_pos[0] = interp(start_pos[0], end_pos[0], (float)t / duration);
            mouse_pos[1] = interp(start_pos[1], end_pos[1], (float)t / duration);
            
            mouse_vel[0] = mouse_pos[0] - last_mouse_pos[0];
            mouse_vel[1] = mouse_pos[1] - last_mouse_pos[1];

            for (int i = 0; i < 3; i++) {
                if (i == action) {
                    mouse_buttons[i] = true;
                } else {
                    mouse_buttons[i] = false;
                }
            }
            
            if (t > duration) {
                for (int i = 0; i < 3; i++) {
                    mouse_buttons[i] = false;
                }
                duration = LENGTH_MIN + (rand() % (LENGTH_MAX - LENGTH_MIN));
                last_time = time;
                moving = false;
            }
        }
    }
    last_mouse_pos[0] = mouse_pos[0];
    last_mouse_pos[1] = mouse_pos[1];
}

void Mouse::resize() {
    SDL_GetWindowSize(window,window_size,window_size+1);
}

void Mouse::getPos(int* wr) {
    wr[0] = (int)(mouse_pos[0] / window_size[0] * grid_w);
    wr[1] = (int)(grid_h - mouse_pos[1] / window_size[1] * grid_h);
}

void Mouse::getVel(float* wr) {
    wr[0] = mouse_vel[0] / window_size[0] * grid_w;
    wr[1] = -(mouse_vel[1] / window_size[1] * grid_h);
}

void Mouse::getButtons(int* wr) {
    wr[0] = mouse_buttons[0];
    wr[1] = mouse_buttons[1];
    wr[2] = mouse_buttons[2];
}