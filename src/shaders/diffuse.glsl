#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputField;
layout(rgba32f, binding = 1) uniform image2D outputField;

uniform int redblack;
uniform float rate;
uniform float dt;

void main() {
    ivec2 dims = imageSize(outputField);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    int offset = int(mod(redblack + id.y,2));
    id.x *= 2;
    id.x += offset;

    float a = dt*rate*dims.x*dims.y;
    vec4 current = texelFetch(inputField,id,0);
    ivec2 p = clamp(id + ivec2(-1,0), ivec2(0), dims-1);
    vec4 l = imageLoad(outputField,p);
    p = clamp(id + ivec2(0,1), ivec2(0), dims-1);
    vec4 u = imageLoad(outputField,p);
    p = clamp(id + ivec2(1,0), ivec2(0), dims-1);
    vec4 r = imageLoad(outputField,p);
    p = clamp(id + ivec2(0,-1), ivec2(0), dims-1);
    vec4 d = imageLoad(outputField,p);

    imageStore(outputField,id,(current + 0.992*a*(l+u+r+d))/(1+4*a));
}