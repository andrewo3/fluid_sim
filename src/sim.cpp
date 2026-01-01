#include "sim.hpp"
#include <gl/glew.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <string>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>
#include "mouse.hpp"



//initialize density textures
void Fluid::initDensTex(GLuint* id_arr) {
    glGenTextures(num_dens_textures, id_arr);

    float init_dens[(const int)(grid_w*grid_h*4)];
    for (int i = 0; i < grid_w*grid_h*4; i++) {
        init_dens[i] = 0.0f;
    }

    for (int i = 0; i < num_dens_textures; i++) {
        glBindTexture(GL_TEXTURE_2D, id_arr[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_w, grid_h, 0, GL_RGBA, GL_FLOAT, init_dens);
    
        //linear interpolation between pixels and clamping on edge
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

GLuint Fluid::initVelTex() {
    float init_vel[grid_w * grid_h * 4] = {0.0};
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_w, grid_h, 0, GL_RGBA, GL_FLOAT, init_vel);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}

Fluid::Fluid(int gridw, int gridh, float diff_rate, float visc) {
    //set grid size
    grid_w = gridw;
    grid_h = gridh;

    //set diffusion rate
    diffRate = diff_rate;

    //set viscosity
    viscosity = visc;
    
    //in vel texture
    VID = initVelTex();
    
    //out vel texture
    V2ID = initVelTex();

    //setup density texture(s)
    initDensTex(densIDs); //in
    initDensTex(dens2IDs); //out

    dens_in = densIDs;
    dens_out = dens2IDs;

    glGenTextures(1, &mixedOut);
    glBindTexture(GL_TEXTURE_2D, mixedOut);

    float init_mixed[(const int)(grid_w*grid_h*4)];
    for (int i = 0; i < grid_w*grid_h*4; i++) {
        init_mixed[i] = 0.0f;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, grid_w, grid_h, 0, GL_RGBA, GL_FLOAT, init_mixed);

    //linear interpolation between pixels and clamping on edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /*if (!addDensity((color_t){.r = 1.0, .g = 0.0, .b = 0.0})) {
        printf("Failed to add new color to fluid.\n");
    }

    if (!addDensity((color_t){.r = 0.0, .g = 1.0, .b = 0.0})) {
        printf("Failed to add new color to fluid.\n");
    }

    if (!addDensity((color_t){.r = 0.0, .g = 0.0, .b = 1.0})) {
        printf("Failed to add new color to fluid.\n");
    }
    if (!addDensity((color_t){.r = 0.0, .g = 1.0, .b = 1.0})) {
        printf("Failed to add new color to fluid.\n");
    }

    if (!addDensity((color_t){.r = 1.0, .g = 1.0, .b = 0.0})) {
        printf("Failed to add new color to fluid.\n");
    }

    if (!addDensity((color_t){.r = 1.0, .g = 0.0, .b = 1.0})) {
        printf("Failed to add new color to fluid.\n");
    }*/
    addDensity((color_t){.r = 1.0, .g = 0.0, .b = 1.0});
    addDensity((color_t){.r = 1.0, .g = 0.0, .b = 0.5});
    addDensity((color_t){.r = 1.0, .g = 0.0, .b = 0.0});
    addDensity((color_t){.r = 1.0, .g = 0.5, .b = 0.0});
    addDensity((color_t){.r = 1.0, .g = 1.0, .b = 0.0});

    //create shaders and programs
    sourceShader.init("shaders/source.glsl");
    sourceProgram.init();
    sourceProgram.attachShader(&sourceShader);
    if (!sourceProgram.link()) {
        printf("Failed to link source sim program.\n");
        printProgramLog(sourceProgram.id);
    }

    diffuseShader.init("shaders/diffuse.glsl");
    diffuseProgram.init();
    diffuseProgram.attachShader(&diffuseShader);
    if (!diffuseProgram.link()) {
        printf("Failed to link diffuse sim program.\n");
        printProgramLog(diffuseProgram.id);
    }

    advectShader.init("shaders/advect.glsl");
    advectProgram.init();
    advectProgram.attachShader(&advectShader);
    if (!advectProgram.link()) {
        printf("Failed to link advect sim program.\n");
        printProgramLog(advectProgram.id);
    }

    project1Shader.init("shaders/project1.glsl");
    project1Program.init();
    project1Program.attachShader(&project1Shader);
    if (!project1Program.link()) {
        printf("Failed to link project1 sim program.\n");
        printProgramLog(project1Program.id);
    }

    project2Shader.init("shaders/project2.glsl");
    project2Program.init();
    project2Program.attachShader(&project2Shader);
    if (!project2Program.link()) {
        printf("Failed to link project2 sim program.\n");
        printProgramLog(project2Program.id);
    }

    project3Shader.init("shaders/project3.glsl");
    project3Program.init();
    project3Program.attachShader(&project3Shader);
    if (!project3Program.link()) {
        printf("Failed to link project3 sim program.\n");
        printProgramLog(project3Program.id);
    }

    boundShader.init("shaders/bound.glsl");
    boundProgram.init();
    boundProgram.attachShader(&boundShader);
    if (!boundProgram.link()) {
        printf("Failed to link bound sim program.\n");
        printProgramLog(boundProgram.id);
    }

    mixShader.init("shaders/mix.glsl");
    mixProgram.init();
    mixProgram.attachShader(&mixShader);
    if (!mixProgram.link()) {
        printf("Failed to link mix sim program.\n");
        printProgramLog(mixProgram.id);
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

bool Fluid::removeDensity(int index) {
    if (index >= densities || densities == 0) {
        return false;
    }
    //shift everything after the index down, overwriting what was there
    for (int i = index+1; i < densities; i++) {
        colors[i-1] = colors[i];
    }
    densities--;
    return true;
}

void Fluid::mixDensities() {
    for (int i = 0; i < num_dens_textures; i++) {
        float mat[12] = {0};
        for (int c = 0; c < 4; c++) {
            int idx = 4*i + c;
            if (idx < MAX_DENSITIES) {
                mat[c*3 + 0] = colors[idx].r;
                mat[c*3 + 1] = colors[idx].g;
                mat[c*3 + 2] = colors[idx].b;
            }
        }
        glUseProgram(mixProgram.id);
        GLint colorsLoc = glGetUniformLocation(mixProgram.id, "colors");
        glUniformMatrix4x3fv(colorsLoc,1,GL_FALSE,mat);

        GLint initLoc = glGetUniformLocation(mixProgram.id, "init");
        glUniform1i(initLoc, i==0);

        //input textures setup
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,dens_out[i]);

        glBindImageTexture(1, mixedOut, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glDispatchCompute((GLuint)grid_w/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }
}

void Fluid::sourceStep(int* mouse_pos_i, float* mouse_vel, int* mouse_buttons, int component, float strength, int brush_size, GLuint input, GLuint output) {
    //use program
    glUseProgram(sourceProgram.id);

    GLint componentLoc = glGetUniformLocation(sourceProgram.id, "component");
    glUniform1i(componentLoc,component);

    //add mouse data as uniforms
    GLint posLoc = glGetUniformLocation(sourceProgram.id, "mouse_pos");
    GLint velLoc = glGetUniformLocation(sourceProgram.id, "mouse_vel");
    GLint buttonsLoc = glGetUniformLocation(sourceProgram.id, "mouse_buttons");
    GLint dtLoc = glGetUniformLocation(sourceProgram.id, "dt");
    GLint srcLoc = glGetUniformLocation(sourceProgram.id, "source_strength");
    GLint brushLoc = glGetUniformLocation(sourceProgram.id, "brush_size");
    glUniform2iv(posLoc,1,mouse_pos_i);
    glUniform2fv(velLoc,1,mouse_vel);
    glUniform3iv(buttonsLoc,1,mouse_buttons);
    glUniform1f(dtLoc,dt);
    glUniform1f(srcLoc,strength);
    glUniform1i(brushLoc,brush_size);

    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,input);

    glBindImageTexture(1, output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute((GLuint)grid_w/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
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

    glDispatchCompute((GLuint)(grid_w/2 + (grid_w%2))/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

}

void Fluid::advectStep(GLuint input, GLuint output, GLuint vel) {
    glUseProgram(advectProgram.id);
    GLint dtLoc = glGetUniformLocation(advectProgram.id, "dt");
    glUniform1f(dtLoc, dt);

    //input textures setup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,input);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D,vel);

    glBindImageTexture(1, output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute((GLuint)grid_w/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

//1st step of mass conserving procedure
void Fluid::project1Step(GLuint input, GLuint scratch) {
    glUseProgram(project1Program.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,input);

    glBindImageTexture(1, scratch, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    glDispatchCompute((GLuint)grid_w/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Fluid::project2Step(int redblack, GLuint scratch) {
    glUseProgram(project2Program.id);
    glBindImageTexture(0, scratch, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    GLint redblackLoc = glGetUniformLocation(project2Program.id, "redblack");
    glUniform1i(redblackLoc, redblack);

    glDispatchCompute((GLuint)(grid_w/2 + (grid_w%2))/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Fluid::project3Step(GLuint scratch, GLuint output) {
    glUseProgram(project3Program.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,scratch);  

    glBindImageTexture(1, output, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute((GLuint)grid_w/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Fluid::boundStep(int type, GLuint inOut) {
    glUseProgram(boundProgram.id);
    glBindImageTexture(0, inOut, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    GLint typeLoc = glGetUniformLocation(boundProgram.id, "type");
    glUniform1i(typeLoc, type);

    glDispatchCompute((GLuint)grid_w/LOCAL_GROUP_SIZE,(GLuint)grid_h/LOCAL_GROUP_SIZE,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Fluid::simStep(Mouse* mouse, float dt_) {
    dt = dt_;
    float SRC_STRENGTH = 300.0;
    float BRUSH_SIZE = 1;
    //get mouse pos, vel, and buttons

    int mouse_pos[2];
    float mouse_vel[2];
    int mouse_buttons[3];
    mouse->getPos(mouse_pos);
    mouse->getVel(mouse_vel);
    mouse->getButtons(mouse_buttons);


    //VELOCITY WORK
    //----------------------------------------------------
    //by the end of the last iteration, VID is scratch work. We want to copy V2ID to VID here:
    glCopyImageSubData(
        V2ID, GL_TEXTURE_2D, 0, 0, 0, 0,
        VID, GL_TEXTURE_2D, 0, 0, 0, 0,
        grid_w, grid_h, 1
    );

    sourceStep(
        mouse_pos,
        mouse_vel,
        mouse_buttons,
        -1, 
        5.0,
        BRUSH_SIZE,
        VID,
        V2ID
    );
    GLuint tmp = V2ID;
    V2ID = VID;
    VID = tmp;

    for (int i = 0; i < GAUSS_SEIDEL_ITERS; i++) {
        //run diffuse step for velocity
        //update in checkerboard pattern
        diffuseStep(0, viscosity, VID, V2ID); // red
        diffuseStep(1, viscosity, VID, V2ID); // black
        boundStep(1,VID);
    }

    project1Step(V2ID, VID); // swapped because the output of the last shader is the read-only input for this one
    boundStep(0,VID);
    
    for (int i = 0; i < GAUSS_SEIDEL_ITERS; i++) {
        //update in checkerboard pattern
        project2Step(0, VID); // red
        project2Step(1, VID); // black
        boundStep(0,VID);
    }

    project3Step(VID, V2ID);
    boundStep(1,V2ID);

    //swap in and out velocities
    tmp = V2ID;
    V2ID = VID;
    VID = tmp;

    advectStep(VID,V2ID,VID);
    boundStep(1,V2ID);

    project1Step(V2ID, VID); // swapped because the output of the last shader is the read-only input for this one
    boundStep(0,VID);

    for (int i = 0; i < GAUSS_SEIDEL_ITERS; i++) {
        //update in checkerboard pattern
        project2Step(0, VID); // red
        project2Step(1, VID); // black
        boundStep(0,VID);
    }

    project3Step(VID, V2ID);
    boundStep(1,V2ID);
    

    //DENSITY WORK
    //--------------------------------------------
    //run source step
    sourceStep(
        mouse_pos,
        mouse_vel,
        mouse_buttons,
        mouse_density % 4, 
        SRC_STRENGTH,
        BRUSH_SIZE,
        dens_in[mouse_density/4],
        dens_out[mouse_density/4]
    );

    //swap in and out densities
    GLuint* tmp2 = dens_out;
    dens_out = dens_in;
    dens_in = tmp2;

    for (int i = 0; i < GAUSS_SEIDEL_ITERS; i++) {
        //run diffuse step for every color
        for (int d = 0; d < (densities + 3) / 4; d++) {
            //update in checkerboard pattern
            diffuseStep(0, diffRate, dens_in[d], dens_out[d]); // red
            diffuseStep(1, diffRate, dens_in[d], dens_out[d]); // black
            boundStep(0,dens_out[d]);
        }
    }

    //swap in and out densities
    tmp2 = dens_out;
    dens_out = dens_in;
    dens_in = tmp2;

    for (int d = 0; d < (densities + 3) / 4; d++) {
        //update in checkerboard pattern
        advectStep(dens_in[d], dens_out[d], V2ID);
        boundStep(0,dens_out[d]);
    }

    mixDensities();
}

Fluid::~Fluid() {
}