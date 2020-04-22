#version 150

in vec4 fs_UV;

out vec4 out_Col;

uniform sampler2D u_sampler1;

void main()
{
    vec4 col = texture(u_sampler1, vec2(fs_UV.x, fs_UV.y));
    out_Col = col;
}
