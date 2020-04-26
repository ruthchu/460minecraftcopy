#version 150

out float fragmentdepth;

uniform sampler2D u_sampler1;

void main()
{
    fragmentdepth = gl_FragCoord.z;
}
