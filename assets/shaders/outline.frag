#version 430 core

in vec2 v_tex_coord;
out vec4 o_frag_color;

uniform sampler2D u_tex;

uniform vec2 u_texel_size;

uniform int u_outline_thickness;
uniform vec4 u_outline_color;

void main() {
    vec4 col = texture(u_tex, v_tex_coord);

    if (col.a > 0.0) {
        // Center isn't transparent, so just return the original color.
        o_frag_color = col;
        return;
    }

    // Check surrounding pixels for a non-transparent neighbor, outline if one exists.
    bool outline = false;

    for (int oy = -u_outline_thickness; oy <= u_outline_thickness; oy++) {
        for (int ox = -u_outline_thickness; ox <= u_outline_thickness; ox++) {
            if (ox == 0 && oy == 0) {
                continue;
            }

            vec2 offs = vec2(ox, oy) * u_texel_size;
            float neighbor_alpha = texture(u_tex, v_tex_coord + offs).a;

            if (neighbor_alpha > 0.0) {
                outline = true;
                break;
            }
        }
    }

    if (outline) {
        o_frag_color = u_outline_color;
    }
}
