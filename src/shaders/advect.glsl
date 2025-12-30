#version 430

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0) uniform sampler2D inputField;
layout(rgba32f, binding = 1) uniform image2D outputField;
layout(binding = 2) uniform sampler2D velField;

uniform float dt;


//advection algorithm from Jos Stam paper, adapted for parallel compute shader
void main() {
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputField);
    float dtx = dt*dims.x;
    float dty = dt*dims.y;
    
    vec2 vel = texelFetch(velField,id,0).xy;
    //position plus scaled velocity clamped to remain within bounds
    vec2 p = clamp(id-vec2(dtx*vel.x,dty*vel.y),vec2(0.5,0.5),vec2(dims.x-0.5,dims.y-0.5));
    ivec2 p0 = ivec2(p); //closest integer cell
    ivec2 p1 = p0 + ivec2(1,1);
    
    vec2 dp1 = p - vec2(p0);
    vec2 dp0 = vec2(1,1) - dp1;

    vec4 f0 = texelFetch(inputField,p0,0);
    vec4 f1 = texelFetch(inputField,ivec2(p0.x,p1.y),0);
    vec4 f2 = texelFetch(inputField,ivec2(p1.x,p0.y),0);
    vec4 f3 = texelFetch(inputField,p1,0);

    vec4 vc1 = dp0.x*(dp0.y*f0+dp1.y*f1)+dp1.x*(dp0.y*f2+dp1.y*f3);
    imageStore(outputField,id,vc1); 

}