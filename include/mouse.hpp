#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <SDL3/SDL.h>

#define LENGTH_MIN 500
#define LENGTH_MAX 3000

class Mouse {
    public:
        Mouse(bool autom, SDL_Window* win, int gw, int gh);
        SDL_Window* window;
        bool automated;
        void getPos(int* wr);
        void getVel(float* wr);
        void getButtons(int* wr);
        void resize();
        void update();
        float interp(int start, int end, float t);

        float mouse_pos[2];
        float mouse_vel[2];
        int mouse_buttons[3];

        int window_size[2];
        int grid_w;
        int grid_h;

        //automated info
        bool moving = false;
        int action = 0;
        int duration = LENGTH_MIN;
        long last_time = 0;
        int start_pos[2];
        int end_pos[2];
        float last_mouse_pos[2] = {0.0};
};

#endif