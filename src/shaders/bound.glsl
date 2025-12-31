#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D inputField;

uniform int type;

void main() {
    ivec2 dims = imageSize(inputField);
    ivec2 id = ivec2(gl_GlobalInvocationID.xy);
    
    vec4 write = vec4(0.0);
    //if on any border
    if (id.x == 0 || id.x == dims.x-1 || id.y == 0 || id.y == dims.y-1) {
        if (type == 0) {
            write = vec4(0.0,0.0,0.0,0.0);
        } else if (type == 1) {
            if (id.x == 0 && id.y > 0 && id.y < dims.y-1) {
                write = imageLoad(inputField,ivec2(id.x+1,id.y));
                write.x = -write.x;
            } else if (id.x == dims.x-1 && id.y > 0 && id.y < dims.y-1) {
                write = imageLoad(inputField,ivec2(id.x-1,id.y));
                write.x = -write.x;
            } else if (id.y == 0 && id.x > 0 && id.x < dims.x-1) {
                write = imageLoad(inputField,ivec2(id.x,id.y+1));
                write.y = -write.y;
            }  else if (id.y == dims.y-1 && id.x > 0 && id.x < dims.x-1) {
                write = imageLoad(inputField,ivec2(id.x,id.y-1));
                write.y = -write.y;
            } else {
                vec4 sum1 = vec4(0.0);
                vec4 sum2 = vec4(0.0);
                if (id == ivec2(0,0)) {
                    sum1 = imageLoad(inputField,ivec2(0,1));
                    sum2 = imageLoad(inputField,ivec2(1,0));
                } else if (id == ivec2(dims.x-1,0)) {
                    sum1 = imageLoad(inputField,ivec2(dims.x-1,1));
                    sum2 = imageLoad(inputField,ivec2(dims.x-2,0));
                } else if (id == ivec2(0,dims.y-1)) {
                    sum1 = imageLoad(inputField,ivec2(0,dims.y-2));
                    sum2 = imageLoad(inputField,ivec2(1,dims.y-1));
                } else if (id == ivec2(dims.x-1,dims.y-1)) {
                    sum1 = imageLoad(inputField,ivec2(dims.x-1,dims.y-2));
                    sum2 = imageLoad(inputField,ivec2(dims.x-2,dims.y-1));
                }
                write = 0.5*(sum1+sum2);
            }
        }
    } else {
        write = imageLoad(inputField,id);
    }
    imageStore(inputField,id,write);
}