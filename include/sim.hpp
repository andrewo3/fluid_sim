#ifndef SIM_HPP
#define SIM_HPP

#include "shader.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <vector>
#include <SDL3/SDL.h>

#define MAX_DENSITIES 8 //should ideally be a multiple of 4 to make RGBA textures out of
#define GAUSS_SEIDEL_ITERS 20

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

class Fluid {
    public:
        Fluid(int gridw, int gridh, float diff_rate);
        ~Fluid();
        bool addDensity(color_t dens_color);
        void initDensTex(GLuint* id_arr);
        GLuint initVelTex();

        void simStep(SDL_Window* window);
        void diffuseStep(int redblack, float rate, GLuint input, GLuint output);
        void sourceStep(SDL_Window* window);
        void advectStep(GLuint input, GLuint output);

        static const int num_dens_textures = (MAX_DENSITIES + 3) / 4;

        GLuint VID;
        GLuint V2ID;
        GLuint densIDs[num_dens_textures];
        GLuint dens2IDs[num_dens_textures];

        GLuint* dens_in;
        GLuint* dens_out;

        Shader sourceShader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram sourceProgram;

        Shader diffuseShader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram diffuseProgram;

        Shader advectShader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram advectProgram;

        int grid_w;
        int grid_h;

        unsigned long currentTime = 0;
        unsigned long lastTime = 0;

        float diffRate = 0.0;

        float dt = 0.0;
        int densities = 0;
        
        color_t colors[MAX_DENSITIES];
};

#endif