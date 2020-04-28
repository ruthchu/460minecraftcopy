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

uniform int u_Time; // A time value that changes once every tick

uniform int u_enviorment;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_UV;
in vec4 fs_PosLight;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

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

        // Check enviroment
//       if (u_enviorment == 1) {
//           finCol.g = clamp(finCol.g * 1.5f, 0.f, 1.f);
//           finCol.b = clamp(finCol.b * 2.2f, 0.7f, 1.f);
//       } else if (u_enviorment == 2) {
//           finCol.r = clamp(finCol.r * 2.3f, 0.7f, 1.f);
//           finCol.g = clamp(finCol.g * 1.3f, 0.f, 1.f);
//       }

       // Draw shadows

       // To NDC [-1,1]
       vec3 shadowCoord = fs_PosLight.xyz / fs_PosLight.w;

       // To [0,1] for sampling
       shadowCoord = shadowCoord * 0.5 + 0.5;

       // Get shadow mapped stored depth
       float storedDepth = texture(u_ShadowMap, shadowCoord.xy).r;
       float fragmentDepth = shadowCoord.z;
//       float bias = max(0.05 * (1.0 - dot(fs_Nor, u_Eye)), 0.005);

       // Check if fragment is in shadow with bias
       float bias = 0;
       bool isInShadow = fragmentDepth - bias > storedDepth;

//       if (isInShadow) {
//           // magenta
//           finCol = vec4(1, 0, 1, 1);
//       } else {
//           // cyan
//           finCol = vec4(0, 1, 1, 1);
//       }

        finCol = vec4(storedDepth, storedDepth, storedDepth, 1.0);
        out_Col = finCol;
}
