#version 330 core

// Task 16: Create a UV coordinate in variable

in vec2 uv;

// Task 8: Add a sampler2D uniform

uniform sampler2D sampler;

// Task 29: Add a bool on whether or not to filter the texture

uniform bool pixelFilter;
uniform bool kernelFilter;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate

    fragColor = texture(sampler, uv);

    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true

    if (kernelFilter) {

        vec4 blurColor = vec4(0.0);

        vec2 increment = 1.f / vec2(textureSize(sampler, 0));

        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                vec2 offset = vec2(i, j) * increment;
                blurColor += texture(sampler, uv + offset) * (1.f/25.f);
            }
        }
        fragColor = blurColor;
    }

    if (pixelFilter) {
        fragColor.r = 1.0 - fragColor.r;
        fragColor.g = 1.0 - fragColor.g;
        fragColor.b = 1.0 - fragColor.b;
    }

}
