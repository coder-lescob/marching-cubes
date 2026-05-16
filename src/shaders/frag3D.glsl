#version 460

in vec2 uv;
in vec3 normal;
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.0, 0.0) * (dot(normal, vec3(0.0, -1.0, 0.0)) * 0.5 + 0.75);
}