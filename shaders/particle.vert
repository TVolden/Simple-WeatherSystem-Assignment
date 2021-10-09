#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in float size;
uniform mat4 model;
uniform float gravity_offset;
uniform float wind_offset;
uniform float particle_density;

out vec4 vtxColor;

void main()
{
    vec3 position = pos + vec3(wind_offset, -gravity_offset, 0) * size * particle_density;

    while (position.y < 0)
        position.y = position.y + 1;
    while (position.y > 1)
        position.y = position.y + 1;

    while (position.z < 0)
        position.z = position.z + 1;
    while (position.z > 1)
        position.z = position.z - 1;

    while (position.x < 0)
        position.x = position.x + 1;
    while (position.x > 1)
        position.x = position.x - 1;

    gl_Position = model * vec4(position, 1.0);
    // Adjust size of particle based on distance from center
    float dist = pow(position.x-0.5, 2) + pow(position.y-0.5, 2); // power to keep positive
    gl_PointSize = size * 1/(1+dist);
    vtxColor = vec4(1.0, 1.0, 1.0, 1.0);
}