#version 330 core
layout (location = 0) in vec3 pos; // xyz position of the particle
layout (location = 1) in float size; // The size of the particle

uniform mat4 worldModel; // The model to world translation matrix
uniform mat4 viewModel; // The world to view translation matrix
uniform float gravity_offset; // Gravity simulation factor
uniform float wind_offset; // Side wind simulation factor
uniform float particle_density; // How much the wind and gravity affects a particle
uniform vec3 perspective; // The position of the camera

out vec4 vtxColor; // Color output for the next shader

const float BOXSIZE = 1;

void main()
{
    // Setup a "velocity" vector to simulate gravity and wind
    vec3 offset = vec3(wind_offset, -gravity_offset, 0) * size * particle_density;
    vec3 position = mod(pos + offset, BOXSIZE);

    // Apply model to world matrix and keep it around for later
    vec4 pos_w = worldModel * vec4(position, 1.0);
    // Apply the world to view matrix and pass it to OpenGL
    gl_Position = viewModel * pos_w;

    // Now let's use the world position to adjust size of particle based on distance from camera position
    // An approximative distance is calculated without the use of square root, this is sufficient for our need
    float dist = pow(pos_w.x - perspective.x, 2.0) + pow(pos_w.y - perspective.y, 2.0) + pow(pos_w.z - perspective.z, 2.0); // power to keep positive

    // Scale particle size down. The distance is downscaled to compensate for not using square root.
    gl_PointSize = size / (1 + dist * 0.01);

    // The particles are hardcoded to white
    vtxColor = vec4(1.0, 1.0, 1.0, 1.0);
}