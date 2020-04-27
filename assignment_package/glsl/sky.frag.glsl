#version 150
uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
// from our CPU, but it's named u_ViewProj so we don't
// have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform float u_Time;

out vec4 out_Col;

const float FAR_CLIP = 1000.0;
const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// sky flavors
#define SUNSET_THRESHOLD 0.75
#define DUSK_THRESHOLD -0.1

// sun parameters
const vec4 sunDir = normalize(vec4(0.5, 1, 0.75, 0));
const float sunSize = 20.0;
const float sunCoreSize = 7.5;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
vec3(254, 192, 81) / 255.0,
vec3(255, 137, 103) / 255.0,
vec3(253, 96, 81) / 255.0,
vec3(57, 32, 51) / 255.0);
// Dusk palette
const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
vec3(96, 72, 120) / 255.0,
vec3(72, 48, 120) / 255.0,
vec3(48, 24, 96) / 255.0,
vec3(0, 24, 72) / 255.0);

const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 cloudColor = sunset[3];

/* Piecewise function which returns some linearly interpolated sunset
    values based on the input */
vec3 toSunset(float y) {
    if(y < 0.5) {
        return sunset[0];
    }
    else if(y < 0.55) {
        return mix(sunset[0], sunset[1], (y - 0.5) / 0.05);
    }
    else if(y < 0.6) {
        return mix(sunset[1], sunset[2], (y - 0.55) / 0.05);
    }
    else if(y < 0.65) {
        return mix(sunset[2], sunset[3], (y - 0.6) / 0.05);
    }
    else if(y < 0.75) {
        return mix(sunset[3], sunset[4], (y - 0.65) / 0.1);
    }
    return sunset[4];
}

vec3 toDusk(float y) {
    if(y < 0.5) {
        return dusk[0];
    }
    else if(y < 0.55) {
        return mix(dusk[0], dusk[1], (y - 0.5) / 0.05);
    }
    else if(y < 0.6) {
        return mix(dusk[1], dusk[2], (y - 0.55) / 0.05);
    }
    else if(y < 0.65) {
        return mix(dusk[2], dusk[3], (y - 0.6) / 0.05);
    }
    else if(y < 0.75) {
        return mix(dusk[3], dusk[4], (y - 0.65) / 0.1);
    }
    return dusk[4];
}

/* Map 3d point to polar coordinates */
vec2 sphereToUV(vec3 p) {
    // x, z axes represent the horizontal plane of the player, phi rotation about y
    float phi = atan(p.z, p.x);
    // phi: [-pi/2,pi/2] => [0,two_pi]
    if(phi < 0) {
        phi += TWO_PI;
    }
    // theta is second degree of freedom based on phi
    // theta: [0, pi]
    float theta = acos(p.y);
    // in the end we get phi and theta: [0,1]
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

void main()
{
    // We want to project our pixel to a plane in our viewing frustum
    // First we transform fragment coords in pixel space to position in world space

    // Transform fragment coords from pixel space to screen space mapped to [-1,1]
    vec2 ndc = ((gl_FragCoord.xy / vec2(u_Dimensions)) - 0.5) * 2.0;

    vec4 p = vec4(ndc, 1, 1); // Pixel at the far clip plane
    p *= FAR_CLIP; // Screen spae -> unhomogenized screen space
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    // To get direction of ray, we "draw" a line from player's eye to the recently
    // porjected point (taking the difference of positions gets us this)
    vec3 rayDir = normalize(p.xyz - u_Eye);

    // convert ray to 2d uv coords
    vec2 uv = sphereToUV(rayDir /*- clamp(sin(u_Eye * 0.01), 0.f, .75f)*/);

    float skyInput = uv.y;
    vec3 sunsetCol = toSunset(skyInput);
    vec3 duskCol = toDusk(skyInput);

    vec3 col = sunsetCol;
    // recall the definition of a dot product. The angle between two normalized vectors
    // can be found by taking the arccos(a dot b). theta: [0,pi]. Multiply by 2 to get [0,two_pi]
    float raySunDot = dot(rayDir, sunDir.xyz);
    float angle = acos(raySunDot) * 2.f * (180.f / PI);

    if (angle < sunSize) {
        if (angle < sunCoreSize) {
            // clear center of the sun
            col = sunColor;
        } else {
            // sun corona mix with color in the sky
            // interpolate sunCoresize as 0% sunset color and 100% on the furthest reaches
            float coronaDist = (angle - sunCoreSize) / (sunSize - sunCoreSize);
            coronaDist = smoothstep(0.0, 1.0, coronaDist);
            col = mix(sunColor, col, coronaDist);
        }
    } else {
        if (raySunDot > DUSK_THRESHOLD) {
            col = col;
        } else if (raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            col = mix(col, duskCol, t);
        } else {
            col = duskCol;
        }
    }

    out_Col = vec4(col, 1);
}
