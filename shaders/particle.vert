#version 330 core
layout (location = 0) in vec3 pos; // xyz position of the particle
layout (location = 1) in float size; // The size of the particle

uniform mat4 viewModel; // The world to view translation matrix
uniform float gravity_offset; // Gravity simulation factor
uniform float wind_offset; // Side wind simulation factor
uniform float particle_density; // How much the wind and gravity affects a particle
uniform vec3 camPos; // The position of the camera
uniform vec3 camForward; // The camera forward offset

out vec3 vtxColor; // Color output for the next shader

const float BOXSIZE = 30;

void main()
{
    // Setup a "velocity" vector to simulate gravity and wind
    vec3 offset = vec3(wind_offset, -gravity_offset, 0) * size * particle_density;
    vec3 boxSize = vec3(BOXSIZE);
    offset -= camPos + camForward + boxSize * 0.5;
    vec3 position = mod(pos + offset, BOXSIZE);
    position += camPos + camForward - boxSize * 0.5;

    // Apply the view matrix and pass it to OpenGL
    gl_Position = viewModel * vec4(position, 1.0);

    // Set size adjusted with distance to particle
    gl_PointSize = size / (1 + distance(position, camPos));

    // The particles are hardcoded to white
    vtxColor = vec3(1.0, 1.0, 1.0);
}