#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D scratchField; //x is p, y is div

uniform int redblack;

void main() {
    ivec2 dims = imageSize(scratchField);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    int offset = int(mod(redblack + id.y,2));
    id.x *= 2;
    id.x += offset;

    ivec2 p = clamp(id + ivec2(-1,0), ivec2(0), dims-1);
    vec4 l = imageLoad(scratchField,p);
    p = clamp(id + ivec2(0,1), ivec2(0), dims-1);
    vec4 u = imageLoad(scratchField,p);
    p = clamp(id + ivec2(1,0), ivec2(0), dims-1);
    vec4 r = imageLoad(scratchField,p);
    p = clamp(id + ivec2(0,-1), ivec2(0), dims-1);
    vec4 d = imageLoad(scratchField,p);

    vec4 current = imageLoad(scratchField,id);
    current.x = (current.y + l.x + u.x + r.x + d.x)/4; // set new p for this iteration

    imageStore(scratchField,id,current);
}