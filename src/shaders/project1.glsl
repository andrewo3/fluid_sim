#version 430

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D inputField;
layout(rgba32f, binding = 1) uniform image2D scratchField; //x is p, y is div

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(scratchField);
    float hx = 1.0/float(dims.x);
    float hy = 1.0/float(dims.y);
    vec2 t1 = texelFetch(inputField,clamp(id+ivec2(1,0),ivec2(0),dims-1),0).xy;
    vec2 t2 = texelFetch(inputField,clamp(id+ivec2(-1,0),ivec2(0),dims-1),0).xy;
    vec2 t3 = texelFetch(inputField,clamp(id+ivec2(0,1),ivec2(0),dims-1),0).xy;
    vec2 t4 = texelFetch(inputField,clamp(id+ivec2(0,-1),ivec2(0),dims-1),0).xy;
    float wr = -0.5*(hx*(t1.x-t2.x)+hy*(t3.y-t4.y));
    imageStore(scratchField,id,vec4(0.0,wr,0.0,0.0)); //p,div,unused,unused
}