#ifndef SIM_HPP
#define SIM_HPP

#include "shader.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <vector>
#include <SDL3/SDL.h>

#define MAX_DENSITIES 8 //should ideally be a multiple of 4 to make RGBA textures out of
#define GAUSS_SEIDEL_ITERS 20
#define LOCAL_GROUP_SIZE 16

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

class Fluid {
    public:
        Fluid(int gridw, int gridh, float diff_rate, float visc);
        ~Fluid();
        bool addDensity(color_t dens_color);
        void initDensTex(GLuint* id_arr);
        GLuint initVelTex();

        void simStep(SDL_Window* window, float dt);
        void diffuseStep(int redblack, float rate, GLuint input, GLuint output);
        void sourceStep(int* mouse_pos_i, float* mouse_vel, int* mouse_buttons, int component, float strength, int brush_size, GLuint input, GLuint output);
        void advectStep(GLuint input, GLuint output, GLuint vel);
        void project1Step(GLuint input, GLuint scratch);
        void project2Step(int redblack, GLuint scratch);
        void project3Step(GLuint scratch, GLuint output);
        void boundStep(int type, GLuint inOut);

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

        Shader project1Shader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram project1Program;

        Shader project2Shader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram project2Program;

        Shader project3Shader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram project3Program;

        Shader boundShader = Shader(GL_COMPUTE_SHADER);
        ShaderProgram boundProgram;

        int grid_w;
        int grid_h;

        float diffRate = 0.0;
        float viscosity = 0.0;

        float dt = 0.0;
        int densities = 0;

        int mouse_density = 0;
        
        color_t colors[MAX_DENSITIES];
};

#endif