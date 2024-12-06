#version 330 core

out vec4 fragColor;

in vec3 worldPos;
in vec3 worldNorm;

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

const vec3 FogBaseColor = vec3(1., 1., 1.);

float calculateFogFactor(float distance) {
    // Exponential fog calculation
    float fogFactor = exp(-fogDensity * distance);
    return clamp(fogFactor, 0.0, 1.0);
}



vec3 applyFog(in vec3 rgb, in float distance, in vec3 rayOri, in vec3 rayDir) {
    float b = fogDensity;

    float fogAmount = b * exp(-rayOri.y * b) *
                      (1.0 - exp(-distance * rayDir.y * b)) /
                      max(abs(rayDir.y), 0.0001);

    fogAmount = clamp(fogAmount, 0.0, 1.0);

    return mix(rgb, fogColor.rgb, fogAmount);
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
            float distance = length(worldPos - vec3(camPos));
            float height = worldPos.y; // Use the world position height

            vec3 finalColor = fragColor.rgb;
            // finalColor = applyFog(finalColor, distance, height);

            finalColor = applyFog(
                finalColor,
                distance,
                vec3(camPos.x, fogHeight, camPos.z),  // Use a base height
                V
            );
            fragColor = vec4(finalColor, 1.0);
        }
    }
}
