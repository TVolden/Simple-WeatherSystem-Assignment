#version 330 core
layout (location = 0) in vec3 pos; // xyz position of the particle
layout (location = 1) in float size; // The size of the particle

uniform mat4 viewModel; // The world to view translation matrix
uniform mat4 preViewModel; // The previous view translation matrix
uniform vec3 velocity; // Used to simulate wind and gravity
uniform float offset; // Used to simulate wind and gravity
uniform float particle_density; // How much the wind and gravity affects a particle
uniform vec3 camPos; // The position of the camera
uniform vec3 camForward; // The camera forward offset
out vec3 vtxColor; // Color output for the next shader

const float BOXSIZE = 30;


void main()
{
    // Setup a "velocity" vector to simulate gravity and wind
    vec3 vel = velocity * offset * size * particle_density;
    vec3 boxSize = vec3(BOXSIZE);
    vel -= camPos + camForward + boxSize * 0.5;
    vec3 position = mod(pos + vel, BOXSIZE);
    position += camPos + camForward - boxSize * 0.5;

    vec4 bottom = viewModel * vec4(position, 1.0);
    vec4 top = preViewModel * vec4(position - velocity * particle_density, 1.0);

    gl_Position = mix(top, bottom, mod(gl_VertexID, 2));

    // Set size adjusted with distance to particle
    //gl_PointSize = size / (1 + distance(position, camPos));

    // The particles are hardcoded to white
    vtxColor = vec3(1.0, 1.0, 1.0);
}