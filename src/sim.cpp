#include "sim.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <stdio.h>

Fluid::Fluid(int gridw, int gridh) {
    //set grid size
    grid_w = gridw;
    grid_h = gridh;

    //initialize velocities arrays
    v_x = new float[gridw*gridh]; 
    v_y = new float[gridw*gridh];
    glGenTextures(1, &VXID);
    glGenTextures(1, &VYID);

    //x texture setup
    glBindTexture(GL_TEXTURE_2D, VXID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, gridw, gridh, 0, GL_RED, GL_FLOAT, v_x);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //y texture setup
    glBindTexture(GL_TEXTURE_2D, VYID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, gridw, gridh, 0, GL_RED, GL_FLOAT, v_y);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    //glBindImageTexture(0, VXID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
}


void Fluid::addDensity(Color dens_color) {
    float* newDensity = new float[grid_w*grid_h];
    GLuint newID;
    glGenTextures(1, &newID);

    //density texture setup
    glBindTexture(GL_TEXTURE_2D, newID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid_w, grid_h, 0, GL_RED, GL_FLOAT, newDensity);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    densIDs.push_back(newID);
    densities.push_back(newDensity);
    colors.push_back(dens_color);
}


Fluid::~Fluid() {
    if (v_x != nullptr) {
        delete[] v_x;
    }

    if (v_y != nullptr) {
        delete[] v_y;
    }

    for (float* ptr: densities) {
        delete[] ptr;
    }
}