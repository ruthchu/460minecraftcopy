#version 150
uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
// from our CPU, but it's named u_ViewProj so we don't
// have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

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

float random1(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);

    // Four corners in 2D of a tile
    float a = random1(i);
    float b = random1(i + vec2(1.0, 0.0));
    float c = random1(i + vec2(0.0, 1.0));
    float d = random1(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define NUM_OCTAVES 5

float fbm(vec2 uv) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5),
                    -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(uv);
        uv = rot * uv * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

/* Warping using fbm. f(p) -> f(g(p)) -> f(p + h(p)) */
float warpFBM(vec2 uv) {
    vec2 q = vec2(fbm(uv), fbm(uv + vec2(1.1, 3.7)));

    vec2 r = vec2(fbm(uv + 1.0 * q + vec2(1.7, 9.2)),
                  fbm(uv + 1.0 * q + vec2(8.3, 2.8)));
    float f = fbm(uv + r);
    if (f < 0.5) {
        f = 1.;
    } else {
        f = 0.;
    }
    return f;
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

//    float skyInput = uv.y;
    vec2 offset = vec2(warpFBM(uv));
    uv = uv + offset * 0.1;

    vec3 sunsetCol = toSunset(uv.y);
    vec3 duskCol = toDusk(uv.y);

    vec3 col = sunsetCol;
    // recall the definition of a dot product. The angle between two normalized vectors
    // can be found by taking the arccos(a dot b). theta: [0,pi]. Multiply by 2 to get [0,two_pi]
    float raySunDot = dot(rayDir, sunDir.xyz);
    float angle = acos(raySunDot) * 2.f * (180.f / PI);

//    u_Time;

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
    } /*else {
        if (raySunDot > DUSK_THRESHOLD) {
            col = col;
        } else if (raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            col = mix(col, duskCol, t);
        } else {
            col = duskCol;
        }
    }*/

    out_Col = vec4(col, 1);
}
