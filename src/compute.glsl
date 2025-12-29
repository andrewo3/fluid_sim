#version 430

const int MAX_DENSITIES = 8;
const int DENS_TEXTURES = (MAX_DENSITIES + 3)/4;

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D vIn;
layout(binding = 1) uniform sampler2D densitiesIn[DENS_TEXTURES];
layout(r32f, binding = 3) uniform image2D vOut;
layout(r32f, binding = 4) uniform image2D densitiesOut[DENS_TEXTURES];


//void diffuse(int N, int b, )


void main() {
    // base pixel color for image
    //addForces();
    //diffuse();
    //move();
}