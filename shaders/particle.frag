#version 330 core
const vec2 CENTER = vec2(0.5, 0.5);
out vec4 FragColor;
in  vec3 vtxColor;

void main() {
    float alpha = (0.5 - distance(gl_PointCoord, CENTER)) / 0.5;
    gl_FragColor = vec4(vtxColor, alpha);
}
