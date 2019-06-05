layout (location = 0) in vec2 p;

uniform vec2 scl;
uniform vec2 offset;
uniform float size;

void main() {
    gl_PointSize = size;
    gl_Position = vec4((p + offset) * scl, 0.0f, 1.0f);
}


