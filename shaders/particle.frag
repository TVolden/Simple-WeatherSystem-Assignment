#version 330 core
const vec2 CENTER = vec2(0.5, 0.5);
out vec4 FragColor;
in  vec3 vtxColor;
in float lenColorScale;

void main() {
    gl_FragColor = vec4(vtxColor, lenColorScale);
}
