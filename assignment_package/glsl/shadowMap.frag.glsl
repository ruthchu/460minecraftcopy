#version 150

in vec4 fs_UV;

out vec4 out_Col;

uniform sampler2D u_sampler1;

void main(void)
{
    vec4 depth = texture(u_sampler1, vec2(fs_UV.x, fs_UV.y));
    vec4 col = vec4(vec3(depth[0]), 1.0);
    out_Col = col;
}
