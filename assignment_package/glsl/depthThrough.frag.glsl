#version 150

//out float fragmentdepth;

uniform sampler2D u_sampler1;

out vec4 out_Col;

void main()
{
    //out_Col = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
    out_Col = vec4(1.0, 0, 0, 1.0);
}
