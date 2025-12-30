#version 430

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D inputField;
layout(rgba32f, binding = 1) uniform image2D outputField;
uniform int component;

uniform ivec2 mouse_pos;
uniform vec2 mouse_vel;
uniform ivec3 mouse_buttons;
uniform float dt;

float source_strength = 20.0;

float getComponent(vec4 v, int i) {
    return (i == 0) ? v.x :
           (i == 1) ? v.y :
           (i == 2) ? v.z :
                      v.w;
}

void setComponent(inout vec4 v, int i, float value) {
    v.x = (i == 0) ? value : v.x;
    v.y = (i == 1) ? value : v.y;
    v.z = (i == 2) ? value : v.z;
    v.w = (i == 3) ? value : v.w;
}

void addSourceFromMouse(ivec2 id) {
    vec4 outv = vec4(0.0,0.0,0.0,0.0);
    vec4 current = texelFetch(inputField,id,0);
    setComponent(outv,component,dt*source_strength);
    imageStore(outputField,id,current + outv);
}


void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputField);

    //right click cell
    if (id == mouse_pos && mouse_buttons.z != 0) {
        addSourceFromMouse(id);
    }
}