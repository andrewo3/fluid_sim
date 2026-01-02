#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D density;
layout(rgba32f, binding = 1) uniform image2D mixed;
uniform mat4x3 colors;
uniform int init;
uniform float hue_shift;

vec3 hueShift(vec3 color, float hue) {
    const vec3 k = vec3(0.57735, 0.57735, 0.57735);
    float cosAngle = cos(hue);
    return vec3(color * cosAngle + cross(k, color) * sin(hue) + k * dot(k, color) * (1.0 - cosAngle));
}

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    vec4 outp = vec4(colors * texelFetch(density,id,0),1.0);
    outp.rgb = hueShift(outp.rgb,hue_shift);

    if (init == 1) {
        imageStore(mixed,id,outp);
    } else {
        vec4 current = imageLoad(mixed,id);
        imageStore(mixed,id,outp+current);
    }
}