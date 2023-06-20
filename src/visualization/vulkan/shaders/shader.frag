#version 450

layout(binding = 1) uniform sampler2D frag_sampler;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 frag_texture;

layout(location = 0) out vec4 output_color;

void main() {
    output_color = texture(frag_sampler, frag_texture);
}
