#version 430

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D scratchField;
layout(rgba32f, binding = 1) uniform image2D outputField;


void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputField);
    float h = 1/length(dims);
    vec2 t1 = imageLoad(outputField,id+ivec2(1,0)).xy;
    vec2 t2 = imageLoad(outputField,id+ivec2(-1,0)).xy;
    vec2 t3 = imageLoad(outputField,id+ivec2(0,1)).xy;
    vec2 t4 = imageLoad(outputField,id+ivec2(0,-1)).xy;
    float wr = -0.5*h*(t1.x-t2.x+t3.y-t4.y);
    imageStore(scratchField,id,vec4(0.0,wr,0.0,0.0)); //p,div,unused,unused
}