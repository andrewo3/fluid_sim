#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <SDL3/SDL.h>

class Mouse {
    public:
        Mouse(bool autom, SDL_Window* win, int gw, int gh);
        SDL_Window* window;
        bool automated;
        void getPos(int* wr);
        void getVel(float* wr);
        void getButtons(int* wr);
        void resize();

        int window_size[2];
        int grid_w;
        int grid_h;
};

#endif