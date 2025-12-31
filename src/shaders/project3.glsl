#version 430

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D scratchField; //x is p, y is div
layout(rgba32f, binding = 1) uniform image2D outputField;

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputField);
    float hx = 1.0/float(dims.x);
    float hy = 1.0/float(dims.y);

    ivec2 p = clamp(id + ivec2(-1,0), ivec2(0), dims-1);
    vec4 l = texelFetch(scratchField,p,0);
    p = clamp(id + ivec2(0,1), ivec2(0), dims-1);
    vec4 u = texelFetch(scratchField,p,0);
    p = clamp(id + ivec2(1,0), ivec2(0), dims-1);
    vec4 r = texelFetch(scratchField,p,0);
    p = clamp(id + ivec2(0,-1), ivec2(0), dims-1);
    vec4 d = texelFetch(scratchField,p,0);

    vec2 current = imageLoad(outputField,id).xy;
    vec2 modify = 0.5*vec2((r.x - l.x)/hx, (u.x - d.x)/hy);
    imageStore(outputField,id,vec4(current - modify,0.0,0.0));
}