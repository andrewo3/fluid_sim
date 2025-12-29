#ifndef SIM_HPP
#define SIM_HPP

#include <gl/glew.h>
#include <vector>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class Fluid {
    public:
        Fluid(int gridw, int gridh);
        Fluid(GLuint vxId,GLuint vyId);
        ~Fluid();
        void addDensity(Color dens_color);
        GLuint VXID;
        GLuint VYID;
        int grid_w;
        int grid_h;
        float* v_x = nullptr;
        float* v_y = nullptr;
        std::vector<float*> densities;
        std::vector<GLuint> densIDs;
        std::vector<Color> colors;
};

#endif