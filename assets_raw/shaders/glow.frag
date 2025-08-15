#version 430 core

in vec2 v_tex_coord;
out vec4 o_frag_color;

uniform sampler2D u_tex;
uniform vec2 u_texel_size;

void main() {
    vec4 tex_col = texture(u_tex, v_tex_coord);
    vec4 tex_col_right = texture(u_tex, v_tex_coord + vec2(u_texel_size.x, 0.0));
    vec4 tex_col_left = texture(u_tex, v_tex_coord + vec2(-u_texel_size.x, 0.0));
    vec4 tex_col_below = texture(u_tex, v_tex_coord + vec2(0.0, u_texel_size.y));
    vec4 tex_col_above = texture(u_tex, v_tex_coord + vec2(0.0, -u_texel_size.y));

    if (tex_col.a == 0.0 && (tex_col_right.a > 0.0 || tex_col_left.a > 0.0 || tex_col_below.a > 0.0 || tex_col_above.a > 0.0)) {
        o_frag_color = vec4(1.0, 1.0, 1.0, 0.5);
    } else {
        o_frag_color = tex_col;
    }
}
