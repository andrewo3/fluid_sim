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
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, grid_w, grid_h, 0, GL_RG, GL_FLOAT, NULL);

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

    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,VID);

    for (int i = 0; i < num_dens_textures; i++) {
        glActiveTexture(GL_TEXTURE1+i);
        glBindTexture(GL_TEXTURE_2D,densIDs[i]);
    }

    int ind = num_dens_textures + 2;
    //output textures setup
    glBindImageTexture(ind, V2ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    for (int i = 0; i < num_dens_textures; i++) {
        glBindImageTexture(ind + i + 2, dens2IDs[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    }

    glDispatchCompute((GLuint)grid_w,(GLuint)grid_h,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

Fluid::~Fluid() {
}