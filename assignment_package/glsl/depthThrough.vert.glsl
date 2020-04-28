#version 150

uniform mat4 u_depthMVP;

uniform mat4 u_LightProj; // orthoprojection

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

const vec3 sunDir = normalize(vec3(0, 0.1, 1.0));

in vec4 vs_Pos;


mat4 lookAt(vec3 eye, vec3 center, vec3 up)
{
    vec3  f = normalize(center - eye);
    vec3  u = normalize(up);
    vec3  s = normalize(cross(f, u));
    u = cross(s, f);
    mat4 result = mat4(1.0);
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] =-f.x;
    result[1][2] =-f.y;
    result[2][2] =-f.z;
    result[3][0] =-dot(s, eye);
    result[3][1] =-dot(u, eye);
    result[3][2] = dot(f, eye);
    return result;
}

vec3 rotateX(vec3 p, float a) {
    mat4 rot = mat4(1, 0, 0, 0,
                    0, cos(a), sin(a), 0,
                    0, -sin(a), cos(a), 0,
                    0, 0, 0, 1);
    vec4 v = rot * vec4(p, 1);
    return v.xyz;
}

void main()
{
    vec3 newSunDir = rotateX(sunDir, u_Time * 0.01);
    newSunDir = normalize(newSunDir);
    vec3 lightPos = -1000 * newSunDir + u_Eye;
    mat4 lightMVP = u_LightProj * lookAt(lightPos, newSunDir, vec3(0, 1, 0));
//    vec4 modelposition = u_Model * vs_Pos;
//    gl_Position = u_depthMVP * u_ViewProj * vs_Pos;
    gl_Position = lightMVP * vs_Pos;
}
