#version 430 core

layout (location = 0) in vec2 a_vert;
layout (location = 1) in vec2 a_tex_coord;

out vec2 v_tex_coord;

uniform vec2 u_pos;
uniform vec2 u_size;
uniform mat4 u_proj;

void main() {
    mat4 model = mat4(
        vec4(u_size.x, 0.0, 0.0, 0.0),
        vec4(0.0, u_size.y, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(u_pos.x, u_pos.y, 0.0, 1.0)
    );

    gl_Position = u_proj * model * vec4(a_vert, 0.0, 1.0);
    v_tex_coord = a_tex_coord;
}
