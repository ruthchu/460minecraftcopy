#version 150

uniform mat4 u_depthMVP;

in vec4 vs_Pos;

void main()
{
//    vec4 modelposition = u_Model * vs_Pos;
//    gl_Position = u_depthMVP * u_ViewProj * vs_Pos;
    gl_Position = u_depthMVP * vs_Pos;
}
