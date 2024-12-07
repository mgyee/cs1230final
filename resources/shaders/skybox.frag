#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform bool fog;
uniform vec4 fogColor;
uniform float fogDensity;


float calculateFogFactor(float distance, float density) {
    float fogFactor = exp(-density * distance);
    return 1.0 - clamp(fogFactor, 0.0, 1.0);
}

vec3 applyFog(vec3 originalColor, vec3 fogColor, float fogFactor) {
    return mix(originalColor, fogColor, fogFactor);
}

void main()
{
    FragColor = vec4(1.0f);
    vec4 skyColor = texture(skybox, TexCoords);

        if (fog) {
            // Calculate distance from camera to skybox
            float distance = length(TexCoords);
            float fogFactor = calculateFogFactor(distance, fogDensity);

            skyColor.rgb = applyFog(skyColor.rgb, fogColor.rgb, fogFactor);
        }

        FragColor = skyColor;
}
