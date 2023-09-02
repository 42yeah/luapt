#version 330 core

uniform bool hasImage;
uniform sampler2D image;
uniform vec2 resolution;

in vec2 uv;

out vec4 color;

void main() {
    float aspect = resolution.x / resolution.y;
    vec2 cuv = vec2(uv.x * aspect, uv.y) * 0.5 + 0.5; // corrected UV
    if (!hasImage) {
        vec2 u = floor(cuv * 10.0);
        int xx = int(u.x + u.y);
        if (xx % 2 == 0) {
            color = vec4(0.9, 0.9, 0.9, 1.0);
        } else {
            color = vec4(0.1, 0.1, 0.1, 1.0);
        }
    } else {
        color = texture(image, cuv);
    }
}
