#ifndef SIM_HPP
#define SIM_HPP

#include "shader.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <vector>

#define MAX_DENSITIES 8 //should be a multiple of 4 to make RGBA textures out of

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

class Fluid {
    public:
        Fluid(int gridw, int gridh);
        Fluid(GLuint vxId,GLuint vyId);
        ~Fluid();
        void addDensity(color_t dens_color);
        void initDensTex(GLuint* id_arr);
        GLuint initVelTex();
        void simStep();

        static const int num_dens_textures = (MAX_DENSITIES + 3) / 4;

        GLuint VID;
        GLuint V2ID;
        GLuint densIDs[num_dens_textures];
        GLuint dens2IDs[num_dens_textures];

        Shader shader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram program;

        int grid_w;
        int grid_h;
        
        color_t colors[MAX_DENSITIES];
};

#endif