#version 330 core

layout(location = 0) in vec3 objPos;
layout(location = 1) in vec3 objNorm;

out vec3 worldPos;
out vec3 worldNorm;

uniform mat4 mvpMat;
uniform mat4 modelMat;
uniform mat4 invTransModelMat;

void main() {
    worldPos = vec3(modelMat * vec4(objPos, 1.0));
    worldNorm = normalize(mat3(invTransModelMat) * objNorm);

    gl_Position = mvpMat * vec4(objPos, 1.0);
}
