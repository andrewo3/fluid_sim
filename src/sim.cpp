#include "sim.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <string>



//initialize density textures
void Fluid::initDensTex(GLuint* id_arr) {
    glGenTextures(num_dens_textures, id_arr);

    for (int i = 0; i < num_dens_textures; i++) {
        glBindTexture(GL_TEXTURE_2D, id_arr[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_w, grid_h, 0, GL_RGBA, GL_FLOAT, NULL);
    
        //linear interpolation between pixels and clamping on edge
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

GLuint Fluid::initVelTex() {
    float init_vel[(const int)(grid_w*grid_h*2)];
    for (int i = 0; i < grid_w*grid_h*2; i++) {
        if (i%2 == 0) {
            init_vel[i] = 0.0f;
        } else {
            init_vel[i] = 1.0f;
        }
    }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_w, grid_h, 0, GL_RG, GL_FLOAT, &init_vel);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}

Fluid::Fluid(int gridw, int gridh) {
    //set grid size
    grid_w = gridw;
    grid_h = gridh;
    
    //in vel texture
    VID = initVelTex();
    
    //out vel texture
    V2ID = initVelTex();

    //setup density texture(s)
    initDensTex(densIDs); //in
    initDensTex(dens2IDs); //out

    dens_in = densIDs;
    dens_out = dens2IDs;

    //create compute shader and program
    shader.init("src/compute.glsl");
    program.init();
    program.attachShader(&shader);
    if (!program.link()) {
        printf("Failed to link sim program.\n");
        printProgramLog(program.id);
    }

    //glBindImageTexture(0, VXID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
}


void Fluid::simStep() {
    glUseProgram(program.id);

    //printf("sim step with %i density textures\n",num_dens_textures);
    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,VID);

    for (int i = 0; i < num_dens_textures; i++) {
        glActiveTexture(GL_TEXTURE1+i);
        glBindTexture(GL_TEXTURE_2D,dens_in[i]);
    }

    int ind = num_dens_textures + 1;
    //output textures setup
    glBindImageTexture(ind, V2ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    for (int i = 0; i < num_dens_textures; i++) {
        glBindImageTexture(ind + i + 1, dens_out[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    }

    glDispatchCompute((GLuint)grid_w,(GLuint)grid_h,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    //swap in and out velocities
    GLuint tmp = V2ID;
    V2ID = VID;
    VID = tmp;

    //swap in and out densities
    GLuint* tmp2 = dens_out;
    dens_out = dens_in;
    dens_in = tmp2;
}

Fluid::~Fluid() {
}