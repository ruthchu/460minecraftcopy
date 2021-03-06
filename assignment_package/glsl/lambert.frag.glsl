#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture; // The texture to be read from by this shader

uniform sampler2D u_ShadowMap; // Shadow map texture read from by labert shader

uniform vec3 u_Eye; // Camera pos

uniform int u_Time; // A time value that changes once every tick
uniform mat4 u_View;

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
// We've written a static matrix for you to use for HW2,
// but in HW3 you'll have to generate one yourself
uniform mat4 u_depthMVP;

uniform ivec2 u_Dimensions; // screen u_Dimensions

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_UV;
in vec4 gl_FragCoord;
in vec4 fs_PosLight;

out vec4 out_Col; // This is the final output color that you will see on your
// screen for the pixel that is currently being processed.

const float FOG_NEAR = 90.1f;
const float FOG_FAR = 100.0f;

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

vec3 fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    vec3 sum = vec3(0.0);
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

vec3 rotateX(vec3 p, float a) {
    mat4 rot = mat4(1, 0, 0, 0,
                    0, cos(a), sin(a), 0,
                    0, -sin(a), cos(a), 0,
                    0, 0, 0, 1);
    vec4 v = rot * vec4(p, 1);
    return v.xyz;
}

const vec4 dayCol = vec4(vec3(114.f, 200.f, 252.f) / 255.f, .25f);
const vec4 nightCol = vec4(vec3(32.f, 24.f, 72.f) / 255.f, .25f);
const vec4 pinkCol = vec4(vec3(255.f, 255.f, 233.f) / 255.f, .25f);
const vec4 yellowCol = vec4(vec3(255.f, 179.f, 208.f) / 255.f, .25f);

void main()
{
    // Material base color (before shading)
    vec4 diffuseColor;
    if (fs_UV.x >= 13.0 / 16.0) {
        // If this block has UV coords that fall in the range of LAVA or WATER
        // offset the UVs as a function of time
        diffuseColor = texture(u_Texture, vec2(fs_UV.x + mod(u_Time / 1000.0, 2.0 / 16.0), fs_UV.y));
    } else {
        // Draw with static UV coords
        diffuseColor = texture(u_Texture, vec2(fs_UV.x, fs_UV.y));
    }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.2;

    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.

    // Compute final shaded color
    vec4 finCol = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);


    // SHADOW ----------------------------------------------------------------------------
    // To NDC (Screen space) [-1,1]
    vec3 shadowCoord = fs_PosLight.xyz / fs_PosLight.w;
    // To [0,1] for sampling
    shadowCoord = shadowCoord * 0.5 + 0.5;
    // Get shadow mapped stored depth
    float storedDepth = texture(u_ShadowMap, shadowCoord.xy).r;
    float fragmentDepth = shadowCoord.z;
    // Check if fragment is in shadow with bias
    float dotNorEye = dot(fs_Nor.xyz, u_Eye);
    float bias = max(0.05 * (1.0 - dotNorEye), 0.005);
    bool isInShadow = storedDepth < fragmentDepth - bias;
    if (isInShadow) {
        finCol.r = clamp(finCol.r - 0.3, 0, 0.3);
        finCol.g = clamp(finCol.g - 0.3, 0, 0.3);
        finCol.b = clamp(finCol.b - 0.3, 0, 0.3);
    }
    out_Col = finCol;
    // finCol = vec4(storedDepth, storedDepth, storedDepth, 1.0);

    // FOG ----------------------------------------------------------------------------
    vec4 camPos = u_View * fs_Pos;

    float depth = -camPos.z;

    // Calculations done to blend the fog color to match the sky color
    float fogMix = normalize(rotateX(normalize(vec3(0, 0.1, 1.0)), u_Time * 0.01)).y;
    float modFogMix = smoothstep(.3f, .6f, (fogMix + 1.f) / 2.f);

    vec4 fogCol = mix(nightCol, yellowCol, modFogMix);
    vec4 secondCol = mix(pinkCol, dayCol, modFogMix);
    fogCol = mix(fogCol, secondCol, modFogMix);
    float fogAmt = smoothstep(FOG_NEAR, FOG_FAR, depth);

    vec4 col = mix(finCol, fogCol, fogAmt);
    out_Col = col;
}
