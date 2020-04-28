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
#define NIGHT_THRESHOLD -.25

// sun parameters
const vec3 sunDir = normalize(vec3(0, 0.1, 1.0));
uniform vec3 u_sunDir;
const float sunSize = 20.0;
const float sunCoreSize = 7.5;
const float hazeDist = 50.0;

// Sunset palette
const vec3 sunset[5] = vec3[](
vec3(255, 229, 119) / 255.0,
vec3(254, 192, 81) / 255.0,
vec3(255, 137, 103) / 255.0,
vec3(253, 96, 81) / 255.0,
vec3(255, 179, 208) / 255.0);
// Dusk palette
const vec3 dusk[5] = vec3[](
vec3(144, 96, 144) / 255.0,
vec3(96, 72, 120) / 255.0,
vec3(72, 48, 120) / 255.0,
vec3(48, 24, 96) / 255.0,
vec3(0, 24, 72) / 255.0);

const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 setSunColor = vec3(254, 225, 171) / 255.0;
const vec3 cloudColor = sunset[3];

/* Piecewise function which returns some linearly interpolated sunset
    values based on the input */
int getIndex(int i, int c) {
    return (i + c) % 5;
}
const int SKY_INDEX = 0;

vec3 toSunset(float y) {
    if(y < 0.5) {
        return sunset[getIndex(SKY_INDEX, 0)];
    }
    else if(y < 0.55) {
        return mix(sunset[getIndex(SKY_INDEX, 0)], sunset[getIndex(SKY_INDEX, 1)], (y - 0.5) / 0.05);
    }
    else if(y < 0.6) {
        return mix(sunset[getIndex(SKY_INDEX, 1)], sunset[getIndex(SKY_INDEX, 2)], (y - 0.55) / 0.05);
    }
    else if(y < 0.65) {
        return mix(sunset[getIndex(SKY_INDEX, 2)], sunset[getIndex(SKY_INDEX, 3)], (y - 0.6) / 0.05);
    }
    else if(y < 0.75) {
        return mix(sunset[getIndex(SKY_INDEX, 3)], sunset[getIndex(SKY_INDEX, 4)], (y - 0.65) / 0.1);
    }
    return sunset[getIndex(SKY_INDEX, 4)];
}

vec3 toDusk(float y) {
    if(y < 0.5) {
        return dusk[getIndex(SKY_INDEX, 0)];
    }
    else if(y < 0.55) {
        return mix(dusk[getIndex(SKY_INDEX, 0)], dusk[getIndex(SKY_INDEX, 1)], (y - 0.5) / 0.05);
    }
    else if(y < 0.6) {
        return mix(dusk[getIndex(SKY_INDEX, 1)], dusk[getIndex(SKY_INDEX, 2)], (y - 0.55) / 0.05);
    }
    else if(y < 0.65) {
        return mix(dusk[getIndex(SKY_INDEX, 2)], dusk[getIndex(SKY_INDEX, 3)], (y - 0.6) / 0.05);
    }
    else if(y < 0.75) {
        return mix(dusk[getIndex(SKY_INDEX, 3)], dusk[getIndex(SKY_INDEX, 4)], (y - 0.65) / 0.1);
    }
    return dusk[getIndex(SKY_INDEX, 4)];
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

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
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

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //  x0 = x0 - 0. + 0.0 * C
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0 );
    vec4 p = permute( permute( permute(
                                   i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
                               + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
                      + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0/7.0; // N=7
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                  dot(p2,x2), dot(p3,x3) ) );
}

float fbm(vec3 p) {
    const int NUM_OCTAVES = 5;
    float v = 0.0;
    float a = 0.5;
    vec3 shift = vec3(100);
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * snoise(p);
        p = p * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

/* Warping using fbm. f(p) -> f(g(p)) -> f(p + h(p)) */
float warpFBM(vec3 p) {
    p += u_Time * 0.01;
    vec3 q = vec3(fbm(p),
                  fbm(p + vec3(1.1, 3.7, 127.1)),
                  fbm(p + vec3(269.5, 183.3, 765.54)));

    vec3 r = vec3(fbm(p + 1.0 * q + vec3(1.7, 9.2, 217.9)),
                  fbm(p + 1.0 * q + vec3(8.3, 2.8, 331.3)),
                  fbm(p + 1.0 * q + vec3(420.69, 631.2,109.21)));
    float f = fbm(p + r);
    return f;
}

float snoiseFBM(vec3 p) {
    p.x += cos(u_Time * 0.001);
    p.y += sin(u_Time * 0.001);
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += snoise(p * freq) * amp;
        freq *= 2;
        amp *= 0.5;
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

vec3 rotateY(vec3 p, float a) {
    mat4 rot = mat4(cos(a), 0, -sin(a), 0,
                    0, 1, 0, 0,
                    sin(a), 0, cos(a), 0,
                    0, 0, 0, 1);
    vec4 v = rot * vec4(p, 1);
    return v.xyz;
}

vec3 rotateZ(vec3 p, float a) {
    mat4 rot = mat4(cos(a), sin(a), 0, 0,
                    -sin(a), cos(a), 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
    vec4 v = rot * vec4(p, 1);
    return v.xyz;
}

void main()
{
    // We want to project our pixel to a plane in our viewing frustum
    // First we transform fragment coords in pixel space to position in world space

    // Transform fragment coords from pixel space to screen space mapped to [-1,1]
    vec2 ndc = ((gl_FragCoord.xy / vec2(u_Dimensions)) - 0.5) * 2.0;

    vec4 p = vec4(ndc, 1, 1); // Pixel at the far clip plane
    p *= FAR_CLIP; //project the pixel to far clip plane
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    // To get direction of ray, we "draw" a line from player's eye to the recently
    // porjected point (taking the difference of positions gets us this)
    vec3 rayDir = normalize(p.xyz - u_Eye);

    // convert ray to 2d uv coords
    vec2 uv = sphereToUV(rayDir);

    vec2 offset = vec2(snoiseFBM(rayDir));
    uv = uv + offset * 0.1;

    vec3 sunsetCol = toSunset(uv.y);
    vec3 duskCol = toDusk(uv.y);
    vec3 nightCol = vec3(32.f, 24.f, 72.f) / 255.f;
    vec3 dayCol = vec3(115.f, 200.f, 252.f) / 255.f;

    vec3 col;
    vec3 sunMoveCol;

    // recall the definition of a dot product. The angle between two normalized vectors
    // can be found by taking the arccos(a dot b). theta: [0,pi]. Multiply by 2 to get [0,two_pi]
    vec3 newSunDir = u_sunDir;
//    vec3 newSunDir = rotateX(sunDir, u_Time * 0.01);
//    newSunDir = normalize(newSunDir);
    float raySunDot = dot(rayDir, newSunDir);
    float angle = acos(raySunDot) * 2.f * (180.f / PI);

    if (WorleyNoise3D(p.xyz) < .02f) {
        nightCol = vec3(252.f, 255.f, 244.f) / 255.f;
    }

    // Base day and night color
    col = mix(nightCol, dayCol, smoothstep(.3f, .5f,(newSunDir.y + 1.f) / 2.f));

    // Mixing in sunset/sunrise and dusk/dawn color surrounding the sun
    // when the sun is near the "horizon line"

    // Mixes the sunset/sunrise and the dusk/dawn color based on the height of the sun
    float haze = smoothstep(-.175f, -.25f, newSunDir.y);
    sunMoveCol = mix(duskCol, sunsetCol, haze);

    // Shrink or grow the size of the mixed sunrise/dawn color as well as change
    // it's visibility based on the height of the sun to ensure a smooth sunrise and sunset
    vec3 haloCol;
    float haloSize;
    float visible;
    if (newSunDir.y < -.05f) {
        visible = smoothstep(-.3f, -.1f, newSunDir.y);
        haloSize = smoothstep(-.3f, -.175f, newSunDir.y);
        haloCol = mix(col, sunMoveCol, visible);
    }
    if (newSunDir.y > -.05f) {
        visible = smoothstep(0.f, .2f, newSunDir.y);
        haloSize = smoothstep(-.25f, .15f, newSunDir.y);
        haloCol = mix(sunMoveCol, col, visible);
    }
    float disk = angle / 360.f * haloSize;
    disk = smoothstep(.7f, 1.f, disk);
    col = mix(haloCol, col, disk);


    // Draw the sun over everything
    if (angle < sunSize) {
        if (angle < sunCoreSize) {
            // clear center of the sun
            if (newSunDir.y < .25f) {
                float weight = smoothstep(-1.f, .25f, newSunDir.y);
                col = mix(setSunColor, sunColor, weight);
            } else {
                col = sunColor;
            }
        } else {
            // sun corona mix with color in the sky
            // interpolate sunCoresize as 0% sunset color and 100% on the furthest reaches
            float coronaDist = (angle - sunCoreSize) / (sunSize - sunCoreSize);
            coronaDist = smoothstep(0.0, 1.0, coronaDist);
            if (newSunDir.y < .25f) {
                float weight = smoothstep(-1.f, .25f, newSunDir.y);
                col = mix(mix(setSunColor, sunColor, weight), col, coronaDist);
            } else {
                col = mix(sunColor, col, coronaDist);
            }
        }
    }
    out_Col = vec4(col, 1);
}
