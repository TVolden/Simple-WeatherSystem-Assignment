#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "glmutils.h"

#include "plane_model.h"
#include "primitives.h"

// structure to hold render info
// -----------------------------
struct SceneObject{
    unsigned int VAO;
    unsigned int vertexCount;
    void drawSceneObject() const{
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES,  vertexCount, GL_UNSIGNED_INT, 0);
    }
};

struct ParticleObject{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int vertexBufferSize;
    void drawParticles() const{
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertexBufferSize);
    }
};

// function declarations
// ---------------------
unsigned int createArrayBuffer(const std::vector<float> &array);
unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array);
unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices);
void setup();
void setupParticles();

void drawObjects(glm::mat4 model);
void drawParticles(glm::mat4 model);
glm::mat4 makeModel(GLFWwindow *window);

// glfw and input functions
// ------------------------
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void drawCube(glm::mat4 model);
void drawPlane(glm::mat4 model);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// global variables used for rendering
// -----------------------------------
SceneObject cube;
SceneObject floorObj;
SceneObject planeBody;
SceneObject planeWing;
SceneObject planePropeller;

ParticleObject weather;
Shader* shaderProgram;
Shader* particleProgram;

// global variables used for control
// ---------------------------------
float currentTime;
float deltaTime;
glm::vec3 camForward(.0f, .0f, -1.0f);
glm::vec3 camPosition(.0f, 1.6f, 0.0f);
float linearSpeed = 0.15f, rotationGain = 30.0f;
const unsigned int particleSize = 4;            // particle attributes
const unsigned int sizeOfFloat = 4;             // bytes in a float
const unsigned int numberOfParticles = 10000;      // number of particles

// Particle Density is useful when we need bigger particles that fall slower.
// 0.25 is good for rain and 0.05 is good for snow.
const float particleDensity = 0.05; // How much the size of a particle is affected by gravity
float gravityOffset = 0; // Used to simulate gravity affecting the particles
float windOffset = 0; // Used to simulate wind affecting the particles

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 5.2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // setup mesh objects
    // ---------------------------------------
    setup();
    setupParticles();

    // set up the z-buffer
    // Notice that the depth range is now set to glDepthRange(-1,1), that is, a left handed coordinate system.
    // That is because the default openGL's NDC is in a left handed coordinate system (even though the default
    // glm and legacy openGL camera implementations expect the world to be in a right handed coordinate system);
    // so let's conform to that
    glDepthRange(-1,1); // make the NDC a LEFT handed coordinate system, with the camera pointing towards +z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 0.02f;
    auto begin = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;
        deltaTime = appTime.count() - currentTime;
        currentTime = appTime.count();

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        // notice that we also need to clear the depth buffer (aka z-buffer) every new frame
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto model = makeModel(window);

        shaderProgram->use();
        drawObjects(model);
        gravityOffset +=  1.0 * deltaTime;
        windOffset += 0.1 * deltaTime;
        particleProgram->use();
        drawParticles(model);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    delete shaderProgram;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

glm::mat4 makeModel(GLFWwindow *window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glm::mat4 projection = glm::perspectiveFov(70.0f, (float)width, (float)height, .01f, 100.0f);
    glm::mat4 view = glm::lookAt(camPosition, camPosition + camForward, glm::vec3(0,1,0));
    return projection * view;
}

void drawParticles(glm::mat4 model) {
    float scale = 40;
    glm::vec3 pos = glm::vec3(-scale/2, 0, -scale/2);
    glm::mat4 world = glm::translate(pos) * glm::scale(scale, scale, scale);
    particleProgram->setMat4("viewModel", model);
    particleProgram->setFloat("gravity_offset", gravityOffset);
    particleProgram->setFloat("wind_offset", windOffset);
    particleProgram->setFloat("particle_density", particleDensity);
    particleProgram->setVec3("camPos", camPosition);
    particleProgram->setVec3("camForward", camForward);
    weather.drawParticles();
}

void drawObjects(glm::mat4 model){
    glm::mat4 scale = glm::scale(1.f, 1.f, 1.f);

    // draw floor (the floor was built so that it does not need to be transformed)
    shaderProgram->setMat4("model", model);
    floorObj.drawSceneObject();

    // draw 2 cubes and 2 planes in different locations and with different orientations
    drawCube(model * glm::translate(2.0f, 1.f, 2.0f) * glm::rotateY(glm::half_pi<float>()) * scale);
    drawCube(model * glm::translate(-2.0f, 1.f, -2.0f) * glm::rotateY(glm::quarter_pi<float>()) * scale);

    drawPlane(model * glm::translate(-2.0f, .5f, 2.0f) * glm::rotateX(glm::quarter_pi<float>()) * scale);
    drawPlane(model * glm::translate(2.0f, .5f, -2.0f) * glm::rotateX(glm::quarter_pi<float>() * 3.f) * scale);
}

void drawCube(glm::mat4 model){
    // draw object
    shaderProgram->setMat4("model", model);
    cube.drawSceneObject();
}

void drawPlane(glm::mat4 model){
    // draw plane body and right wing
    shaderProgram->setMat4("model", model);
    planeBody.drawSceneObject();
    planeWing.drawSceneObject();

    // propeller,
    glm::mat4 propeller = model * glm::translate(.0f, .5f, .0f) *
                          glm::rotate(currentTime * 10.0f, glm::vec3(0.0,1.0,0.0)) *
                          glm::rotate(glm::half_pi<float>(), glm::vec3(1.0,0.0,0.0)) *
                          glm::scale(.5f, .5f, .5f);

    shaderProgram->setMat4("model", propeller);
    planePropeller.drawSceneObject();

    // right wing back,
    glm::mat4 wingRightBack = model * glm::translate(0.0f, -0.5f, 0.0f) * glm::scale(.5f,.5f,.5f);
    shaderProgram->setMat4("model", wingRightBack);
    planeWing.drawSceneObject();

    // left wing,
    glm::mat4 wingLeft = model * glm::scale(-1.0f, 1.0f, 1.0f);
    shaderProgram->setMat4("model", wingLeft);
    planeWing.drawSceneObject();

    // left wing back,
    glm::mat4 wingLeftBack =  model *  glm::translate(0.0f, -0.5f, 0.0f) * glm::scale(-.5f,.5f,.5f);
    shaderProgram->setMat4("model", wingLeftBack);
    planeWing.drawSceneObject();
}

void bindAttributes(){
    int posSize = 3; // each position has x,y
    GLuint vertexLocation = glGetAttribLocation(particleProgram->ID, "pos");
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribPointer(vertexLocation, posSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, 0);

    int sizeSize = 1;
    GLuint vertexSize  = glGetAttribLocation(particleProgram->ID, "size");
    glEnableVertexAttribArray(vertexSize);
    glVertexAttribPointer(vertexSize, sizeSize, GL_FLOAT, GL_FALSE, particleSize * sizeOfFloat, (void*)(3*sizeof(float)));
}

void createVertexBufferObject(){
    glGenVertexArrays(1, &weather.VAO);
    glGenBuffers(1, &weather.VBO);

    glBindVertexArray(weather.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, weather.VBO);

    // initialize particle buffer, set all values to 0
    std::vector<float> data(numberOfParticles * particleSize);
    for(unsigned int i = 0; i < data.size(); i++)
        data[i] = 0.0f;

    // allocate at openGL controlled memory
    glBufferData(GL_ARRAY_BUFFER, numberOfParticles * particleSize * sizeOfFloat, &data[0], GL_DYNAMIC_DRAW);
    bindAttributes();
}

void setupParticles() {
    // initialize particle shaders
    particleProgram = new Shader("shaders/particle.vert", "shaders/particle.frag");
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

    createVertexBufferObject();

    // load particles
    glBindVertexArray(weather.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, weather.VBO);

    // Split the space into fractions
    float frag = cbrt(numberOfParticles);

    /*
     * To evenly populate the volume evenly, every cubic fragment is populated with a particle,
     * some randomness is added to make it look less symmetric.
     */
    for(float i = 0; i < numberOfParticles; i++) {
        float data[particleSize];
        // XYZ position of particle
        data[0] = (rand()/(float)RAND_MAX + fmod(i, frag)) / frag * 30.0; //x;
        data[1] = fmod(rand()/(float)RAND_MAX + i / frag, frag) / frag * 30.0; //y;
        data[2] = (rand()/(float)RAND_MAX + i / glm::pow(frag, 2.0)) / frag * 30.0; //z;

        // size of particle, this influences how much gravity affects the particle
        data[3] = rand() / (float)RAND_MAX * 20.0 + 20.0;

        // Add to buffer
        glBufferSubData(GL_ARRAY_BUFFER, i * particleSize * sizeOfFloat, particleSize * sizeOfFloat, data);
    }

    weather.vertexBufferSize = numberOfParticles;
}

void setup(){
    // initialize shaders
    shaderProgram = new Shader("shaders/shader.vert", "shaders/shader.frag");

    // load floor mesh into openGL
    floorObj.VAO = createVertexArray(floorVertices, floorColors, floorIndices);
    floorObj.vertexCount = floorIndices.size();

    // load cube mesh into openGL
    cube.VAO = createVertexArray(cubeVertices, cubeColors, cubeIndices);
    cube.vertexCount = cubeIndices.size();

    // load plane meshes into openGL
    planeBody.VAO = createVertexArray(planeBodyVertices, planeBodyColors, planeBodyIndices);
    planeBody.vertexCount = planeBodyIndices.size();

    planeWing.VAO = createVertexArray(planeWingVertices, planeWingColors, planeWingIndices);
    planeWing.vertexCount = planeWingIndices.size();

    planePropeller.VAO = createVertexArray(planePropellerVertices, planePropellerColors, planePropellerIndices);
    planePropeller.vertexCount = planePropellerIndices.size();
}


unsigned int createVertexArray(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<unsigned int> &indices){
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // bind vertex array object
    glBindVertexArray(VAO);

    // set vertex shader attribute "pos"
    createArrayBuffer(positions); // creates and bind  the VBO
    int posAttributeLocation = glGetAttribLocation(shaderProgram->ID, "pos");
    glEnableVertexAttribArray(posAttributeLocation);
    glVertexAttribPointer(posAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // set vertex shader attribute "color"
    createArrayBuffer(colors); // creates and bind the VBO
    int colorAttributeLocation = glGetAttribLocation(shaderProgram->ID, "color");
    glEnableVertexAttribArray(colorAttributeLocation);
    glVertexAttribPointer(colorAttributeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // creates and bind the EBO
    createElementArrayBuffer(indices);

    return VAO;
}


unsigned int createArrayBuffer(const std::vector<float> &array){
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, array.size() * sizeof(GLfloat), &array[0], GL_STATIC_DRAW);

    return VBO;
}


unsigned int createElementArrayBuffer(const std::vector<unsigned int> &array){
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, array.size() * sizeof(unsigned int), &array[0], GL_STATIC_DRAW);

    return EBO;
}

// NEW!
// instead of using the NDC to transform from screen space you can now define the range using the
// min and max parameters
void cursorInRange(float screenX, float screenY, int screenW, int screenH, float min, float max, float &x, float &y){
    float sum = max - min;
    float xInRange = (float) screenX / (float) screenW * sum - sum/2.0f;
    float yInRange = (float) screenY / (float) screenH * sum - sum/2.0f;
    x = xInRange;
    y = -yInRange; // flip screen space y axis
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    // TODO - rotate the camera position based on mouse movements
    //  if you decide to use the lookAt function, make sure that the up vector and the
    //  vector from the camera position to the lookAt target are not collinear
    float x, y;
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    cursorInRange(posX, posY, width, height, 0, 360, x, y);

    auto lookAround = glm::rotateY(glm::radians(-x / rotationGain));
    auto lookUpDown = glm::rotateX(glm::radians(glm::max(glm::min(y / rotationGain, 89.0f), -89.0f)));
    camForward = lookAround * lookUpDown * glm::vec4 (0,0,-1, 1);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    auto direction = camForward * glm::vec3(linearSpeed, 0, linearSpeed);
    auto rotateY = glm::mat3(0, 0, -1, 0, 1, 0, 1, 0, 0);
    // TODO move the camera position based on keys pressed (use either WASD or the arrow keys)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camPosition += direction;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camPosition -= direction;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camPosition += direction * rotateY;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camPosition -= direction * rotateY;
    }

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}