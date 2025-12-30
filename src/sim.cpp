#include "sim.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <string>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>



//initialize density textures
void Fluid::initDensTex(GLuint* id_arr) {
    glGenTextures(num_dens_textures, id_arr);

    float init_dens[(const int)(grid_w*grid_h*4)];
    for (int i = 0; i < grid_w*grid_h*4; i++) {
        init_dens[i] = 0.0f;
    }

    for (int i = 0; i < num_dens_textures; i++) {
        glBindTexture(GL_TEXTURE_2D, id_arr[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_w, grid_h, 0, GL_RGBA, GL_FLOAT, &init_dens);
    
        //linear interpolation between pixels and clamping on edge
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

GLuint Fluid::initVelTex() {
    float init_vel[(const int)(grid_w*grid_h*4)];
    for (int i = 0; i < grid_w*grid_h*4; i++) {
        if (i%4 == 1) {
            init_vel[i] = 1.0f;
        } else {
            init_vel[i] = 0.0f;
        }
    }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_w, grid_h, 0, GL_RGBA, GL_FLOAT, &init_vel);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}

Fluid::Fluid(int gridw, int gridh, float diff_rate) {
    //set grid size
    grid_w = gridw;
    grid_h = gridh;

    //set diffusion rate
    diffRate = diff_rate;
    
    //in vel texture
    VID = initVelTex();
    
    //out vel texture
    V2ID = initVelTex();

    //setup density texture(s)
    initDensTex(densIDs); //in
    initDensTex(dens2IDs); //out

    dens_in = densIDs;
    dens_out = dens2IDs;

    if (!addDensity((color_t){.r = 255, .g = 0, .b = 0})) {
        printf("Failed to add new color to fluid.\n");
    }

    //create shaders and programs
    sourceShader.init("src/shaders/source.glsl");
    sourceProgram.init();
    sourceProgram.attachShader(&sourceShader);
    if (!sourceProgram.link()) {
        printf("Failed to link source sim program.\n");
        printProgramLog(sourceProgram.id);
    }

    diffuseShader.init("src/shaders/diffuse.glsl");
    diffuseProgram.init();
    diffuseProgram.attachShader(&diffuseShader);
    if (!diffuseProgram.link()) {
        printf("Failed to link diffuse sim program.\n");
        printProgramLog(diffuseProgram.id);
    }

    advectShader.init("src/shaders/advect.glsl");
    advectProgram.init();
    advectProgram.attachShader(&advectShader);
    if (!advectProgram.link()) {
        printf("Failed to link advect sim program.\n");
        printProgramLog(advectProgram.id);
    }
}

bool Fluid::addDensity(color_t color) {
    if (densities >= MAX_DENSITIES) {
        return false;
    }
    colors[densities] = color;
    densities++;
    return true;
}

void Fluid::sourceStep(SDL_Window* window) {
    //get mouse pos, vel, and buttons
    float mouse_pos[2];
    float mouse_vel[2];
    SDL_MouseButtonFlags buttons = SDL_GetMouseState(mouse_pos,mouse_pos+1);
    SDL_MouseButtonFlags buttons2 = SDL_GetRelativeMouseState(mouse_vel,mouse_vel+1);

    int window_size[2];
    SDL_GetWindowSize(window,window_size,window_size+1);

    int mouse_buttons[3] = {
        buttons & SDL_BUTTON_LMASK,
        buttons & SDL_BUTTON_MMASK,
        buttons & SDL_BUTTON_RMASK
    };

    int mouse_pos_i[2];

    mouse_pos_i[0] = (int)(mouse_pos[0] / window_size[0] * grid_w);
    mouse_pos_i[1] = (int)(grid_h - mouse_pos[1] / window_size[1] * grid_h);

    mouse_vel[0] = mouse_vel[0] / window_size[0] * grid_w;
    mouse_vel[1] = mouse_vel[1] / window_size[1] * grid_h;

    int mouse_density = 0; // indicate which density to use

    //use program
    glUseProgram(sourceProgram.id);

    GLint componentLoc = glGetUniformLocation(sourceProgram.id, "component");
    glUniform1i(componentLoc,mouse_density % 4);

    //add mouse data as uniforms
    GLint posLoc = glGetUniformLocation(sourceProgram.id, "mouse_pos");
    GLint velLoc = glGetUniformLocation(sourceProgram.id, "mouse_vel");
    GLint buttonsLoc = glGetUniformLocation(sourceProgram.id, "mouse_buttons");
    GLint dtLoc = glGetUniformLocation(sourceProgram.id, "dt");
    glUniform2iv(posLoc,1,mouse_pos_i);
    glUniform2fv(velLoc,1,mouse_vel);
    glUniform3iv(buttonsLoc,1,mouse_buttons);
    glUniform1f(dtLoc,dt);

    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,dens_in[mouse_density/4]);

    glBindImageTexture(1, dens_out[mouse_density/4], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute((GLuint)grid_w,(GLuint)grid_h,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    //swap in and out densities
    GLuint* tmp2 = dens_out;
    dens_out = dens_in;
    dens_in = tmp2;
}

void Fluid::diffuseStep(int redblack, float rate, GLuint input, GLuint output) {
    glUseProgram(diffuseProgram.id);
    GLint redblackLoc = glGetUniformLocation(diffuseProgram.id, "redblack");
    GLint rateLoc = glGetUniformLocation(diffuseProgram.id, "rate");
    GLint dtLoc = glGetUniformLocation(diffuseProgram.id, "dt");
    glUniform1i(redblackLoc, redblack);
    glUniform1f(rateLoc, rate);
    glUniform1f(dtLoc, dt);

    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,input);

    glBindImageTexture(1, output, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute((GLuint)(grid_w/2 + (grid_w%2)),(GLuint)grid_h,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

}

void Fluid::advectStep(GLuint input, GLuint output) {
    glUseProgram(advectProgram.id);
    GLint dtLoc = glGetUniformLocation(advectProgram.id, "dt");
    glUniform1f(dtLoc, dt);

    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,input);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,VID);

    glBindImageTexture(1, output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute((GLuint)grid_w,(GLuint)grid_h,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Fluid::simStep(SDL_Window* window) {
    //calculate dt
    currentTime = SDL_GetTicks();
    dt = (currentTime - lastTime)/1000.0;
    lastTime = currentTime;

    //run source step
    sourceStep(window);

    float diffRate = 0.001;
    for (int i = 0; i < GAUSS_SEIDEL_ITERS; i++) {
        //run diffuse step for every color
        for (int d = 0; d < (densities + 3) / 4; d++) {
            //update in checkerboard pattern
            diffuseStep(0, diffRate, dens_in[d / 4], dens_out[d / 4]); // red
            diffuseStep(1, diffRate, dens_in[d / 4], dens_out[d / 4]); // black
        }
    }

    //swap in and out densities
    GLuint* tmp2 = dens_out;
    dens_out = dens_in;
    dens_in = tmp2;

    for (int d = 0; d < (densities + 3) / 4; d++) {
        //update in checkerboard pattern
        advectStep(dens_in[d / 4], dens_out[d / 4]); // red
    }
    /*
    //swap in and out velocities
    GLuint tmp = V2ID;
    V2ID = VID;
    VID = tmp;
    //swap in and out densities
    GLuint* tmp2 = dens_out;
    dens_out = dens_in;
    dens_in = tmp2;*/
}

Fluid::~Fluid() {
}