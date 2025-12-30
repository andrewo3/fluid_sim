#version 430

layout(location = 0) in vec2 LVertexPos2D;
layout(location = 1) in vec2 textureCoordinates;

out vec2 interpolatedTextureCoordinates;

void main() {
    interpolatedTextureCoordinates = textureCoordinates; // attribute 1
    gl_Position = vec4(LVertexPos2D, 0.0, 1.0);
}