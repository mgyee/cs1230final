#version 330 core

out vec4 fragColor;

in vec3 worldPos;
in vec3 worldNorm;

uniform vec4 ambient;

uniform vec4 diffuse;
uniform vec4 specular;

uniform vec4 lightPos;

uniform float shininess;
uniform vec4 camPos;

struct Light
{
  int type;
  vec3 pos;
  vec3 dir;
  vec4 color;
};

uniform Light lights[8];
uniform int lightsCount;

void main() {
    fragColor = ambient;
    vec3 N = normalize(worldNorm);
    vec3 V = normalize(vec3(camPos) - worldPos);

    for (int i = 0; i < lightsCount; i++) {
        Light light = lights[i];
        // vec3 L = normalize(vec3(lightPos) - worldPos);
        vec3 L = -normalize(light.dir);

        vec3 R = normalize(reflect(-L, N));
        float spec = clamp(dot(R, V), 0.0, 1.0);
        if (spec >= 0 || (spec == 0 && shininess > 0)) {
            spec = pow(spec, shininess);
        }
        fragColor += light.color * (diffuse * clamp(dot(N, L), 0.0, 1.0) + specular * spec);
    }
}
