#version 430

const int MAX_DENSITIES = 8;
const int DENS_TEXTURES = (MAX_DENSITIES + 3)/4;

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D vIn;
layout(binding = 1) uniform sampler2D densitiesIn[DENS_TEXTURES];
layout(rg32f, binding = 3) uniform image2D vOut;
layout(rgba32f, binding = 4) uniform image2D densitiesOut[DENS_TEXTURES];


float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

//void diffuse(int N, int b, )


void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(vOut);

    // Normalize to [0,1] for visualization
    vec2 uv = vec2(id) / vec2(dims);

    // Output gradient: R = x, G = y
    float n = rand(uv+texture(vIn,uv).xy);
    float n2 = rand(uv-texture(vIn,uv).xy);
    vec4 color = vec4(n,n2,0.0,1.0);
    imageStore(vOut, id, color);

    // base pixel color for image
    //addForces();
    //diffuse();
    //move();
}