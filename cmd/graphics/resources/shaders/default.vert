#version 330 core

layout(location = 0) in vec3 objPos;
layout(location = 1) in vec3 objNorm;
layout(location = 2) in vec2 uv;

out vec3 worldPos;
out vec3 worldNorm;
out vec2 outUV;
out vec4 eyeSpacePos;

uniform mat4 mvpMat;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 invTransModelMat;

void main() {
    worldPos = vec3(modelMat * vec4(objPos, 1.0));
    worldNorm = normalize(mat3(invTransModelMat) * objNorm);

    vec4 eyePos = viewMat * modelMat * vec4(objPos, 1.0);
    eyeSpacePos = eyePos;

    outUV = uv;

    gl_Position = mvpMat * vec4(objPos, 1.0);
}
