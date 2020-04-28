#version 150

in vec4 fs_UV;

out vec4 out_Col;

uniform sampler2D u_sampler1;

uniform ivec2 u_Dimensions;

void main(void)
{
    vec2 res = gl_FragCoord.xy / vec2 (u_Dimensions.x, u_Dimensions.y);
    vec4 depth = texture(u_sampler1, res);
    vec4 col = vec4(depth.r, depth.r, depth.r, 1.0);
//    vec4 depth = texture(u_sampler1, vec2(fs_UV.x, fs_UV.y));
//    vec4 col = vec4(vec3(depth.r), 1.0);
    out_Col = col;
}
