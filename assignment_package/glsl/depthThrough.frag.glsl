#version 150

out vec4 fragmentdepth;

uniform sampler2D u_sampler1;

void main()
{
    fragmentdepth = gl_FragCoord.z;
}
