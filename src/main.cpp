// assignment 4: solar system (by WjZhang)

#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb.h>

#include "constants.h"
#include "util.h"
#include "camera.h"
#include "shader_s.h"
#include "filesystem.h"
#include "texture.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
using namespace std;
using namespace glm;

Camera camera(vec3(0.f, 30.f, 90.f), 100.f, -90.f, degrees(atan(- 1.f/ 3.f)));

bool  firstMouse = true;
bool  running = true;
bool  keyPress = false;
float deltaTime  = 0.f;
float nowTime    = 0.f; 
float currFrame  = 0.f;
float lastFrame  = 0.f;
float lastX = SCR_WIDTH  / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

void resizeWindowCallback(GLFWwindow *window, int width, int height);
void mouseMovecallback(GLFWwindow* window, double xpos, double ypos);
void scrollcallback(GLFWwindow* window, double xoffset, double yoffset);
void keyPressHandler(GLFWwindow *window);
void frameHandler();

inline void initOpenGL();

inline void genVAOs(int VAOidx, bool circleOn);
inline void initObjs();
inline void loadData2VBOs(const vector<vec3> &vertexes, const vector<vec3> &norms, 
                const vector<vec2> &uvs, const vector<unsigned int> &indices, bool circleOn);

void genSphere();
inline void genObjects();
inline void cleanup();

inline void xyz2righthand(mat4 &xyz);
inline void objectRotate(mat4 &object);
inline void objectOrbit(mat4 &object);

vec3 SphereCenter;
float SphereRadius;
vector<vec3> SpherePoints;
vector<vec3> SphereNorms;
vector<vec2> SphereUVs;
vector<unsigned int> SphereIndices;

vec3 CircleCenter;
float CircleRadius;
vector<vec3> CirclePoints;
vector<vec3> CircleNorms;
vector<vec2> CircleUVs;
vector<unsigned int> CircleIndices;

vec3 SunCenter;
vec3 PlanetCenter;
vec3 MoonCenter;
vec3 JupiterCenter;

enum IVBO: int {VERTEX, NORM, UV, INDICES};
enum IVAO: int {DEFAULT_VAO, CIRCLE_VAO};
enum TEX:  int {SUN, PLANET, MOON, JUPITER, SPACE};
GLuint VBOs[4]; // VBOIDs
GLuint VAOs[2]; // VAOIDs
GLuint Textures[6]; // TextureIDs

inline void initOpenGL() 
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

inline void initObjs() 
{
    glGenVertexArrays(2, VAOs);
    glGenBuffers(4, VBOs);
}

inline void genVAOs(int VAOidx, bool circleOn) 
{
    glBindVertexArray(VAOs[VAOidx]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::VERTEX]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::NORM]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    if (!circleOn) {
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::UV]);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[IVBO::INDICES]);
}

inline void loadData2VBOs(const vector<vec3> &vertexes, const vector<vec3> &norms, 
                const vector<vec2> &uvs, const vector<unsigned int> &indices, bool circleOn) 
{
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexes.size(), &vertexes[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::NORM]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * norms.size(), &norms[0], GL_STATIC_DRAW);
    
    if (!circleOn) {
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::UV]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[IVBO::INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void genSphere(vector<vec3> &vertexes, vector<vec3> &norms, vector<vec2> &uvs, vector<unsigned int> &indices,
                vec3 center, float radius, int division) 
{
    float step = 1.0f / (float)(division - 1);
    float u = 0.0f;
    float v = 0.0f;
    for (auto i = 0; i < division; ++i) {
        v = 0.0f;
        for (auto j = 0; j < division; ++j) {
            vec3 pos = vec3(radius * cos(2.0f * PI * u) * sin(PI * v),
                            radius * sin(2.0f * PI * u) * sin(PI * v),
                            radius * cos(PI * v)) + center; // world axis (x, y, z)
            vec3 norm = normalize(pos - center); 
            vertexes.push_back(pos);
            norms.push_back(norm);
            uvs.push_back(vec2(u, v));
            v += step;
        }
        u += step;
    }
    for (auto i = 0; i < division - 1; ++i) {
        for (auto j = 0; j < division - 1; ++j) {
            unsigned int p00 = i * division + j;
			unsigned int p01 = i * division + j + 1;
			unsigned int p10 = (i + 1) * division + j;
			unsigned int p11 = (i + 1) * division + j + 1;

			indices.push_back(p00);
			indices.push_back(p10);
			indices.push_back(p01);

			indices.push_back(p01);
			indices.push_back(p10);
			indices.push_back(p11);
        }
    }
}

void genCircle(vector<vec3> &vertexes, vector<vec3> &norms, vector<unsigned int> &indices,
                vec3 center, float inner, float outer, int division)
{
    float step = 2.f / (float)(division);
    float rd = 0.f;
    for (auto i = 0; i < division; ++i) {
        vec3 posi = vec3(inner * cos(rd * PI), inner * sin(rd * PI), 0.f);
        vec3 poso = vec3(outer * cos(rd * PI), outer * sin(rd * PI), 0.f);
        vec3 norm = vec3(0.f, 0.f, 1.f);
        vertexes.push_back(posi);
        vertexes.push_back(poso);
        norms.push_back(norm);
        norms.push_back(norm);
        rd += step;
    }
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(division * 2 - 2);
    
    indices.push_back(division * 2 - 1);
    indices.push_back(1);
    indices.push_back(division * 2 - 2);
    
    for (auto i = 0; i < division - 1; ++i) {
        unsigned int p00 = i << 1;
        unsigned int p01 = p00 + 1;
        unsigned int p10 = p01 + 1;
        unsigned int p11 = p10 + 1;
        
        indices.push_back(p00);
        indices.push_back(p01);
        indices.push_back(p10);

        indices.push_back(p11);
        indices.push_back(p01);
        indices.push_back(p10);
    }
}

inline void genObjects() 
{
    SphereCenter = vec3(0.0);
	SphereRadius = 5.f;
    genSphere(SpherePoints, SphereNorms, SphereUVs, SphereIndices, SphereCenter, SphereRadius, 96);
    CircleCenter = vec3(0.0);
    genCircle(CirclePoints, CircleNorms, CircleIndices, CircleCenter, 1.f, 0.99f, 360);    

    Textures[TEX::SUN] = LoadTexture("resources/textures/sun.jpg");
    Textures[TEX::PLANET] = LoadTexture("resources/textures/planet.jpg");  
    Textures[TEX::SPACE] = LoadTexture("resources/textures/space.png");  
    Textures[TEX::MOON] = LoadTexture("resources/textures/moon.jpg");
    Textures[TEX::JUPITER] = LoadTexture("resources/textures/jupitermap.jpg");
}

inline void render(Shader &shader, mat4 model, mat4 projection, mat4 view, IVAO vaoid, int elements, GLuint texture, GLuint textureUnit, bool diffuseOn, bool circleOn) 
{
    glBindVertexArray(VAOs[vaoid]);
    shader.use(); 
    if (!circleOn) {
        shader.setBool("diffuseOn", diffuseOn);
        shader.setVec3("light.position", vec3(0.f, 0.f, 0.f));
        shader.setVec3("light.diffuse", vec3(.8f, .8f, .8f));
        shader.setVec3("light.ambient", vec3(.2f, .2f, .2f));
        shader.setVec3("viewPos", camera.Position);
        glActiveTexture(textureUnit);
    	glBindTexture(GL_TEXTURE_2D, texture);
    }
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, (void*)0);
}

inline void cleanup() 
{
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(4, VBOs);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "solar", NULL, NULL);
    if (isNull((void*)window)) {
        errLog("Failed to create window");
        glfwTerminate(); exit(-1);
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window ,resizeWindowCallback);
    glfwSetCursorPosCallback(window, mouseMovecallback);
    glfwSetScrollCallback(window, scrollcallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        errLog("Failed to init GLAD");
        glfwTerminate(); exit(-1);
    }

    initOpenGL();
    Shader DefaultShader("resources/default.vert", "resources/default.frag");
    Shader CircleShader("resources/circle.vert", "resources/circle.frag");

    genObjects();

    initObjs();
    genVAOs(IVAO::DEFAULT_VAO, false);
    genVAOs(IVAO::CIRCLE_VAO, true);
    
    while (!glfwWindowShouldClose(window)) {
        frameHandler();
        keyPressHandler(window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render------------------------------------------

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, CAMNEAR, CAMFAR);
        glm::mat4 view = camera.GetViewMatrix();
        
        loadData2VBOs(SpherePoints, SphereNorms, SphereUVs, SphereIndices, false);
        mat4 sunModel = mat4(1.f);
        sunModel = rotate(sunModel, nowTime * .1f, vec3(0.f, 1.f, 0.f));
        sunModel = scale(sunModel, vec3(1.5f, 1.5f, 1.5f));
        xyz2righthand(sunModel);
        render(DefaultShader, sunModel, projection, view, IVAO::DEFAULT_VAO, SphereIndices.size(), Textures[TEX::SUN], GL_TEXTURE0, false, false);
        
        mat4 planetModel = mat4(1.f);
        planetModel = rotate(planetModel, nowTime * .3f, vec3(0.f, 1.f, 0.f));
        planetModel = translate(planetModel, vec3(15.f, 0.f, 0.f));
        planetModel = rotate(planetModel, nowTime * 1.5f, vec3(0.f, 1.f, 0.f));
        planetModel = scale(planetModel, vec3(.3f, .3f, .3f));
        xyz2righthand(planetModel);
        render(DefaultShader, planetModel, projection, view, IVAO::DEFAULT_VAO, SphereIndices.size(), Textures[TEX::PLANET], GL_TEXTURE0, true, false);
        
        mat4 moonModel = mat4(1.f);
        moonModel = rotate(moonModel, nowTime * .3f, vec3(0.f, 1.f, 0.f));
        moonModel = translate(moonModel, vec3(15.f, 0.f, 0.f));
        moonModel = rotate(moonModel, -nowTime * .8f, normalize(vec3(1.f, 8.f, 0.f)));
        moonModel = translate(moonModel, vec3(3.f, -.375f, 0.f));
        moonModel = rotate(moonModel, -atan(1.f/8.f), vec3(0.f, 0.f, 1.f));
        moonModel = rotate(moonModel, -nowTime * .5f + PI, normalize(vec3(0.f, 1.f, 0.f)));
        moonModel = scale(moonModel, vec3(.1f, .1f, .1f));
        xyz2righthand(moonModel);
        render(DefaultShader, moonModel, projection, view, IVAO::DEFAULT_VAO, SphereIndices.size(), Textures[TEX::MOON], GL_TEXTURE0, true, false);

        mat4 jupiterModel = mat4(1.f);
        jupiterModel = rotate(jupiterModel, nowTime * .2f, normalize(vec3(1.f, 6.f, 0.0f)));
        jupiterModel = translate(jupiterModel, vec3(36.f, -6.f, 0.f));
        jupiterModel = rotate(jupiterModel, -atan(1.f/6.f), vec3(0.f, 0.f, 1.f));
        jupiterModel = rotate(jupiterModel, nowTime * 1.f, vec3(0.f, 1.f, 0.f));
        jupiterModel = scale(jupiterModel, vec3(.8f, .8f, .8f));
        xyz2righthand(jupiterModel);
        render(DefaultShader, jupiterModel, projection, view, IVAO::DEFAULT_VAO, SphereIndices.size(), Textures[TEX::JUPITER], GL_TEXTURE0, true, false);

        mat4 spaceModel = mat4(1.f);
        spaceModel = scale(spaceModel, vec3(30.f, 30.f, 30.f));
        xyz2righthand(spaceModel);
        render(DefaultShader, spaceModel, projection, view, IVAO::DEFAULT_VAO, SphereIndices.size(), Textures[TEX::SPACE], GL_TEXTURE0, false, false);
        
        // circles
        loadData2VBOs(CirclePoints, CircleNorms, CircleUVs, CircleIndices, true);
        mat4 jupiterCircle = mat4(1.f);
        jupiterCircle = translate(jupiterCircle, vec3(0.f, -0.4f, 0.f));
        jupiterCircle = rotate(jupiterCircle, -atan(1.f/6.f), vec3(0.f, 0.f, 1.f));
        jupiterCircle = scale(jupiterCircle, vec3(sqrt(36.f * 36.f + 6.f * 6.f), 1.0f, sqrt(36.f * 36.f + 6.f * 6.f)));
        xyz2righthand(jupiterCircle);
        render(CircleShader, jupiterCircle, projection, view, IVAO::CIRCLE_VAO, CircleIndices.size(), 0, 0, false, true);

        mat4 planetCircle = mat4(1.f);
        planetCircle = scale(planetCircle, vec3(15.f, 1.0f, 15.f));
        xyz2righthand(planetCircle);
        render(CircleShader, planetCircle, projection, view, IVAO::CIRCLE_VAO, CircleIndices.size(), 0, 0, false, true);

        mat4 moonCircle = mat4(1.f);
        moonCircle = rotate(moonCircle, nowTime * .3f, vec3(0.f, 1.f, 0.f));
        moonCircle = translate(moonCircle, vec3(15.f, 0.f, 0.f));
        moonCircle = rotate(moonCircle, -atan(1.f/8.f), vec3(0.f, 0.f, 1.f));        
        moonCircle = scale(moonCircle, vec3(3.f, 1.f, 3.f));
        xyz2righthand(moonCircle);
        render(CircleShader, moonCircle, projection, view, IVAO::CIRCLE_VAO, CircleIndices.size(), 0, 0, false, true);
        // ------------------------------------------------

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void frameHandler()
{
    currFrame = glfwGetTime();
    deltaTime = currFrame - lastFrame;
    lastFrame = currFrame;
    if(running) nowTime += deltaTime;
}

void keyPressHandler(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && keyPress == false) {
        running = 1 - running;
        keyPress = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        keyPress = false;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void resizeWindowCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouseMovecallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollcallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

inline void xyz2righthand(mat4 &xyz) 
{
    xyz = rotate(xyz, -(PI / 2.f), vec3(1.f, 0.f, 0.f));
}