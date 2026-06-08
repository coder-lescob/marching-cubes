#version 460

in vec2 uv;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform mat4x4 projection_matrix;

float depth_to_linear_depth(float depth) {
    float far = projection_matrix[2][2] / (projection_matrix[2][1] + 1.0);
    float near = projection_matrix[2][2] / (projection_matrix[2][1] - 1.0);
    return (2.0 * near) / (far + near - depth * (far - near));
}

void main() {
    float depth = depth_to_linear_depth(texture(depthTexture, uv).r);

    vec3 scattering_coefficients = vec3(0.0055, 0.0026, 0.0010) * 75.0;
    vec3 transmittance = vec3(exp(-depth * scattering_coefficients));
    vec3 color = texture(screenTexture, uv).rgb;

    FragColor = vec4(mix(color, vec3(0.0, 0.25, 0.5), transmittance), 1.0);
}