#version 410 core
layout (location = 0) in vec3 vp;
layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec4 worldPos = model * vec4(vp, 1.0);
    FragPos = worldPos.xyz;
    Normal = normalize(mat3(transpose(inverse(model))) * normal);
    gl_Position = projection * view * worldPos;
}