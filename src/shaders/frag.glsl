#version 430

out vec4 LFragment; 
uniform sampler2D textureData;

in vec2 interpolatedTextureCoordinates;

void main() {
    LFragment.rgb = texture(textureData, interpolatedTextureCoordinates).rgb;
    LFragment.a = 1.0;
}