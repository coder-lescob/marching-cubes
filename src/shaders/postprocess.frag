#version 460

in vec2 uv;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main() {
    FragColor = 1 - texture(screenTexture, uv);
}