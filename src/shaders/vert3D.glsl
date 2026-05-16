#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uvs;
layout (location = 2) in vec3 normals;

uniform mat4x4 view_matrix;
uniform mat4x4 projection_matrix;
uniform mat4x4 model_matrix;

out vec2 uv;
out vec3 normal;

void main() {
    gl_Position =  projection_matrix * view_matrix * model_matrix * vec4(pos, 1.0);
    uv = uvs;
    normal = normals;
}