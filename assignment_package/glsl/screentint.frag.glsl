#version 150

in vec4 fs_UV;

out vec4 out_Col;

uniform sampler2D u_sampler1;

void main()
{
    vec4 txCol = texture(u_sampler1, vec2(fs_UV.x, fs_UV.y));
    float blueparam = txCol.b;
    if (blueparam * 1.3 > 1) {
        blueparam = 1.3;
    } else {
        blueparam *= 1.3;
    }
    vec4 col = vec4(txCol.r, txCol.g, blueparam, txCol[3]);
}
