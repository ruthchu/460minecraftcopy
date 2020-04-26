#version 150

uniform mat4 u_depthMVP;

in vec4 vs_Pos;
//in vec4 vs_UV;

void main()
{
//    gl_Position = u_depthMVP * vs_Pos;
    gl_Position = vs_Pos;
}
