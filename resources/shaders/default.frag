#version 330 core

out vec4 fragColor;

in vec3 worldPos;
in vec3 worldNorm;
in vec4 eyeSpacePos;

uniform vec4 ambient;

uniform vec4 diffuse;
uniform vec4 specular;

uniform float shininess;
uniform vec4 camPos;

uniform bool fog;
uniform vec4 fogColor;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;
uniform float fogHeight;
uniform float fogBaseHeight;

struct Light
{
  int type;
  vec3 pos;
  vec3 dir;
  vec4 color;
  vec3 function;
  float angle;
  float penumbra;
};

uniform Light lights[8];
uniform int lightsCount;


float calculateVolumetricFogFactor(float eyeSpaceDepth, float worldHeight) {
    // float baseFogFactor = 1.0 - exp(-fogDensity * eyeSpaceDepth);
    float baseFogFactor = 1.0 - exp(-pow(fogDensity * eyeSpaceDepth, 2.0));

    float heightFactor = smoothstep(fogBaseHeight, fogHeight, worldHeight);

    float densityFactor = 1.0 - exp(-fogDensity * eyeSpaceDepth);

    // return clamp(baseFogFactor * (1.0 - heightFactor), 0.0, 1.0);
    return clamp(baseFogFactor * (1.0 - heightFactor) * densityFactor, 0.0, 1.0);
}

vec3 calculateFogColor(float worldHeight) {
    vec3 baseFogColor = fogColor.rgb;

    vec3 skyColor = vec3(0.7, 0.8, 1.0);
    vec3 groundColor = vec3(0.8, 0.8, 0.8);

    float heightBlend = smoothstep(fogBaseHeight, fogHeight, worldHeight);
    vec3 dynamicFogColor = mix(groundColor, skyColor, heightBlend);

    return mix(baseFogColor, dynamicFogColor, 0.5);
}

void main() {
    fragColor = ambient;
    vec3 N = normalize(worldNorm);
    vec3 V = normalize(vec3(camPos) - worldPos);
    vec3 position = worldPos;

    for (int i = 0; i < lightsCount; i++) {
        Light light = lights[i];

        float dist;
        vec3 L;
        vec4 I = light.color;

        float f = 1.0;


        if (light.type == 0) {
            L = -normalize(light.dir);
        } else if (light.type == 1) {
            L = normalize(light.pos - position);
            float x = acos(clamp(dot(normalize(light.dir), -L), -1.f, 1.f));
            float outer = light.angle;
            float inner = outer - light.penumbra;

            if (x > outer) {
                I = vec4(0);
            } else if (x > inner) {
                float falloff = -2.f * pow((x - inner) / (outer - inner), 3) + 3.f * pow((x - inner) / (outer - inner), 2);
                I *= (1 - falloff);
            }

            dist = distance(position, light.pos);
            f = min(1.0, 1.0 / (light.function[0] + light.function[1] * dist + light.function[2] * pow(dist, 2)));

        } else {
            dist = distance(position, light.pos);
            L = normalize(light.pos - position);
            f = min(1.0, 1.0 / (light.function[0] + light.function[1] * dist + light.function[2] * pow(dist, 2)));
        }

        vec3 R = normalize(reflect(-L, N));

        float diffuse_closeness = clamp(dot(N, L), 0.0, 1.0);

        float specular_closeness = clamp(dot(R, V), 0.0, 1.0);

        if (specular_closeness >= 0 && (specular_closeness != 0 || shininess > 0)) {
            specular_closeness = pow(specular_closeness, shininess);
        }

        fragColor += f * I * (diffuse * diffuse_closeness + specular * specular_closeness);



        if (fog) {
                float eyeSpaceDepth = abs(eyeSpacePos.z);

                vec3 newFogColor = calculateFogColor(worldPos.y);

                float fogFactor = calculateVolumetricFogFactor(eyeSpaceDepth, worldPos.y);

                fragColor.rgb = mix(fragColor.rgb, newFogColor.rgb, fogFactor);
            }
    }
}
