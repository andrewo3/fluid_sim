#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputField;
layout(rgba32f, binding = 1) uniform image2D outputField;
uniform int component;

uniform ivec2 mouse_pos;
uniform vec2 mouse_vel;
uniform ivec3 mouse_buttons;
uniform float dt;
uniform int brush_size;

uniform float source_strength;

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
    if (component != -1) {
        setComponent(outv,component,dt*source_strength);
    } else {
        outv = vec4(dt*source_strength*(mouse_vel/dt)*0.4,0.0,0.0);
    }
    imageStore(outputField,id,current + outv);
}

void addSinkFromMouse(ivec2 id,ivec2 sink_center) {
    vec4 outv = vec4(0.0,0.0,0.0,0.0);
    vec4 current = texelFetch(inputField,id,0);
    if (component != -1) {
        //setComponent(outv,component,-dt*source_strength);
    } else {
        float d = length(id-sink_center);
        vec2 pull = source_strength*vec2(id-sink_center)/(d*d*20+0.00001);
        outv = vec4(-pull,0.0,0.0);
        imageStore(outputField,id,current + outv);
    }
    
}

void addForceFromMouse(ivec2 id) {
    vec4 outv = vec4(0.0,0.0,0.0,0.0);
    vec4 current = texelFetch(inputField,id,0);
    if (component == -1) { // this is only true when the velocity field calls this
        outv = vec4(dt*source_strength*(mouse_vel/dt)*0.4,0.0,0.0);
    }
    imageStore(outputField,id,current + outv);
}

void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputField);

    //right click cell
    if (length(id - mouse_pos) <= brush_size && mouse_buttons.z != 0) {
        addSourceFromMouse(id);
    } else if (length(id - mouse_pos) <= brush_size*2 && mouse_buttons.x != 0) {
        addForceFromMouse(id);
    } else if (mouse_buttons.y != 0) {
        addSinkFromMouse(id,mouse_pos);
    }
}