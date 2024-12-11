#version 330 core

// Task 16: Create a UV coordinate in variable

in vec2 uv;

// Task 8: Add a sampler2D uniform

uniform sampler2D sampler;

// Task 29: Add a bool on whether or not to filter the texture

uniform bool pixelFilter;
uniform bool kernelFilter;

uniform bool FXAAEnabled;

out vec4 fragColor;

// FXAA Constants
const float EDGE_THRESHOLD_MIN = 0.0312;
const float EDGE_THRESHOLD_MAX = 0.125;
const float SUBPIXEL_QUALITY = 0.75;
const int ITERATIONS = 12;
const float FXAA_REDUCE_MIN = 1.0 / 128.0;
const float FXAA_REDUCE_MUL = 1.0 / 8.0;
const float FXAA_SPAN_MAX = 8.0;
const float NORMALIZING_GRADIENT_FACTOR = 0.25;


float FxaaLuma(vec3 rgb) {
 // return rgb.y * (0.587/0.299) + rgb.x;
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

float increasingStepSizes(int i) {
    float increasingStepSizes[7] = float[](1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0); // increasing step size as i increases to get to edge faster
    return i <= 6 ? increasingStepSizes[i] : 8.0;
}

vec4 fxaa(sampler2D tex, vec2 pos, vec2 textureSize) {
    // Get texture size for pixel calculations
    vec2 inverseTextureSize = 1.0 / textureSize;


    // get luma of north, east, south and west neighbours
    float lumaSouth = FxaaLuma(textureOffset(tex, pos, ivec2(0, -1)).rgb);
    float lumaNorth = FxaaLuma(textureOffset(tex, pos, ivec2(0, 1)).rgb);
    float lumaWest = FxaaLuma(textureOffset(tex, pos, ivec2(-1, 0)).rgb);
    float lumaEast = FxaaLuma(textureOffset(tex, pos, ivec2(1, 0)).rgb);

    // get luma of current pixel
    float lumaCenter = FxaaLuma(textureOffset(tex, pos, ivec2(0,0)).rgb);

    // get luma of diagonal pixels
    float lumaSW = FxaaLuma(textureOffset(tex, pos, ivec2(-1, -1)).rgb);
    float lumaNE = FxaaLuma(textureOffset(tex, pos, ivec2(1, 1)).rgb);
    float lumaNW = FxaaLuma(textureOffset(tex, pos, ivec2(-1, 1)).rgb);
    float lumaSE = FxaaLuma(textureOffset(tex, pos, ivec2(1, -1)).rgb);

    //  get max and min luma from NESW neighbours
    float lumaMin = min(lumaCenter, min(min(lumaSouth, lumaNorth), min(lumaWest, lumaEast)));
    float lumaMax = max(lumaCenter, max(max(lumaSouth, lumaNorth), max(lumaWest, lumaEast)));
    float lumaRange = lumaMax - lumaMin;

    // If less than thresold, no edge so just return pixel
    if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)) {
        return texture(tex, pos);
    }

    // constants for edge information
    float lumaStoN = lumaSouth + lumaNorth;
    float lumaEtoW = lumaEast + lumaWest;
    float lumaLeftCorners = lumaSW + lumaNW;
    float lumaDownCorners = lumaSW + lumaSE;
    float lumaRightCorners = lumaSE + lumaNE;
    float lumaUpCorners = lumaNE + lumaNW;

    // calculate which direction edge is in (horizontal or vertical)
    float horizontalDirection = abs(-2.0 * lumaWest + lumaLeftCorners) +
                           abs(-2.0 * lumaCenter + lumaStoN) * 2.0 +
                           abs(-2.0 * lumaEast + lumaRightCorners);
    float verticalDirection = abs(-2.0 * lumaNorth + lumaUpCorners) +
                         abs(-2.0 * lumaCenter + lumaEtoW) * 2.0 +
                         abs(-2.0 * lumaSouth + lumaDownCorners);

    // if horizontalComponent > verticalComponent, edge is horizontal
    bool isHorizontal = horizontalDirection >= verticalDirection;

    // Select neighboring pixels opposite to edge direction (if horizontal, choose up and down, if vertical, choose left and right)
    float luma1 = isHorizontal ? lumaSouth : lumaWest;
    float luma2 = isHorizontal ? lumaNorth : lumaEast;

    // get gradients
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    // find steepest gradient direction
    bool is1Steepest = abs(gradient1) >= abs(gradient2);

    // normalize gradient
    float gradientScaled = NORMALIZING_GRADIENT_FACTOR * max(abs(gradient1), abs(gradient2));

    // get step length opposite to edge direction
    float stepLength = isHorizontal ? inverseTextureSize.y : inverseTextureSize.x;
    float lumaLocalAverage;

    if (is1Steepest) {

        // negate step length
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    } else {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }

    // shift initial pos by 0.5 pixels
    vec2 currentUV = pos;
    if (isHorizontal) {
        currentUV.y += stepLength * 0.5;
    } else {
        currentUV.x += stepLength * 0.5;
    }

    // explore edge for first iteration

    // get vec2 in direction corresponding to edge
    vec2 offset = isHorizontal ? vec2(inverseTextureSize.x, 0.0) : vec2(0.0, inverseTextureSize.y);

    // get offsets
    vec2 pos1 = currentUV - offset;
    vec2 pos2 = currentUV + offset;

    // get lumas at new pos
    float lumaEnd1 = FxaaLuma(texture(tex, pos1).rgb);
    float lumaEnd2 = FxaaLuma(texture(tex, pos2).rgb);

    // find gradient
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    // Check if greater than scaledGradient. If so, reached end of edge
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    // If not reached end, keep on iterating
    if (!reachedBoth) {
        for (int i = 2; i < ITERATIONS; i++) {

            // if haven't reached in first direction, keep on continouing
            if (!reached1) {
                lumaEnd1 = FxaaLuma(texture(tex, pos1).rgb) - lumaLocalAverage;
            }

            // if haven't reached in second direction, keep on continouing
            if (!reached2) {
                lumaEnd2 = FxaaLuma(texture(tex, pos2).rgb) - lumaLocalAverage;
            }

            // once again, find gradient and check if greater than scaledGradient
            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            // if still not reached, continue to explore
            if (!reached1) {
                pos1 -= offset * increasingStepSizes(i); // increasingStepSizes is used to increase number of pixels we step by
            }
            if (!reached2) {
                pos2 += offset * increasingStepSizes(i);
            }

            if (reachedBoth) {
                break;
            }
        }
    }

    // get distance from each extreme side of edge
    float distance1 = isHorizontal ? (pos.x - pos1.x) : (pos.y - pos1.y);
    float distance2 = isHorizontal ? (pos2.x - pos.x) : (pos2.y - pos.y);


    // get minimum distance
    bool isInFirstDirection = distance1 < distance2;
    float minDistance = min(distance1, distance2);

    // edge thickness
    float edgeThickness = (distance1 + distance2);

    // get new offset
    float pixelOffset = -minDistance / edgeThickness + 0.5;

    // check that the values we got actually makes sense

    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

    // luma at each end should be positive if center is smaller than average
    bool correctVariation = ((isInFirstDirection ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    // if above condtion fails, no offset
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // Sub-pixel anti-aliasing

    // get average of luma over a 3x3 window
    float lumaAverage = (1.0/12.0) * (2.0 * (lumaStoN + lumaEtoW) +
                                       lumaLeftCorners + lumaRightCorners);

    // get subpixel offsets based on ratio between lumAverage and center pixel
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange, 0.0, 1.0);
    float subPixelOffset2 = pow(subPixelOffset1, 2) * (-2.0 * subPixelOffset1 + 3.0);
    float subPixelOffsetFinal = pow(subPixelOffset2, 2) * SUBPIXEL_QUALITY;

    // get max offset
    finalOffset = max(finalOffset, subPixelOffsetFinal);

    // Apply final pos offset in direction perpendicular to edge
    vec2 finalPos = pos;
    if (isHorizontal) {
        finalPos.y += finalOffset * stepLength;
    } else {
        finalPos.x += finalOffset * stepLength;
    }

    return texture(tex, finalPos);
}

void main()
{
    fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate

    if (FXAAEnabled) {
        fragColor = fxaa(sampler, uv, vec2(textureSize(sampler, 0)));
        } else {
            fragColor = texture(sampler, uv);
        }

    // fragColor = texture(sampler, uv);

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
