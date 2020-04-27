#version 150

out float fragmentdepth;
//out vec4 out_Col; // This is the final output color that you will see on your

void main()
{
    fragmentdepth = gl_FragCoord.z;
//    out_Col = vec4(fragmentdepth, fragmentdepth, fragmentdepth, 1.0);
}
