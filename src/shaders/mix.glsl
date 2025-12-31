#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D density;
layout(rgba32f, binding = 1) uniform image2D mixed;
uniform mat4x3 colors;
uniform int init;

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec4 outp = vec4(colors * texelFetch(density,id,0),1.0);

    if (init == 1) {
        imageStore(mixed,id,outp);
    } else {
        vec4 current = imageLoad(mixed,id);
        imageStore(mixed,id,outp+current);
    }
}