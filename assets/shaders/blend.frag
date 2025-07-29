#version 430 core

in vec2 v_tex_coord;
in float v_intensity;
out vec4 o_frag_color;

uniform sampler2D u_tex;
uniform vec3 u_col;

// TODO: This should accept blend intensity.
void main() {
    vec4 tex_col = texture(u_tex, v_tex_coord);

    if (tex_col.a > 0.0) {
        o_frag_color = vec4(u_col, 1.0);
    } else {
        o_frag_color = tex_col;
    }
}
