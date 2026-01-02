#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <chrono>

#define LENGTH_MIN 500
#define LENGTH_MAX 3000

long GetTicks();

extern std::chrono::steady_clock::time_point start;

class Mouse {
    public:
        Mouse(int gw, int gh);
        void getPos(int* wr);
        void getVel(float* wr);
        void getButtons(int* wr);
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