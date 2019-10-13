

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

Camera camera(glm::vec3(0.0f, 0.0f, 50.0f));

bool  firstMouse = true;
float deltaTime  = 0.0f;
float currFrame  = 0.0f;
float lastFrame  = 0.0f;
float lastX = SCR_WIDTH  / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

bool CheckGLErrors(string location);

void resizeWindowCallback(GLFWwindow *window, int width, int height);
void mouseMovecallback(GLFWwindow* window, double xpos, double ypos);
void scrollcallback(GLFWwindow* window, double xoffset, double yoffset);
void keyPressHandler(GLFWwindow *window);
void frameHandler();

inline void initOpenGL();

inline void genVAOs(int VAOidx);
inline void initObjs();
inline void loadData2VBOs(const vector<vec3> &vertexes, const vector<vec3> &norms, 
                const vector<vec2> &uvs, const vector<unsigned int> &indices);

void genSphere();
inline void genObjects();

inline void cleanup();

vec3 SunCenter;
float SunRadius;
vector<vec3> SunPoints;
vector<vec3> SunNorms;
vector<vec2> SunUVs;
vector<unsigned int> SunIndices;
vec3 PlanetCenter;
float PlanetRadius;
vector<vec3> PlanetPoints;
vector<vec3> PlanetNorms;
vector<vec2> PlanetUVs;
vector<unsigned int> PlanetIndices;
vec3 MoonCenter;
float MoonRadius;
vector<vec3> MoonPoints;
vector<vec3> MoonNorms;
vector<vec2> MoonUVs;
vector<unsigned int> MoonIndices;
vec3 JupiterCenter;
float JupiterRadius;
vector<vec3> JupiterPoints;
vector<vec3> JupiterNorms;
vector<vec2> JupiterUVs;
vector<unsigned int> JupiterIndices;
vec3 SpaceCenter;
float SpaceRadius;
vector<vec3> SpacePoints;
vector<vec3> SpaceNorms;
vector<vec2> SpaceUVs;
vector<unsigned int> SpaceIndices;

enum IVBO: int {VERTEX, NORM, UV, INDICES};
enum IVAO: int {DEFAULT, DEBUG};
enum TEX:  int {SUN, PLANET, MOON, JUPITER, SPACE, DEBUG};
GLuint VBOs[4]; // VBOIDs
GLuint VAOs[1]; // VAOIDs
GLuint Textures[6]; // TextureIDs

inline void initOpenGL() 
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

inline void initObjs() 
{
    glGenVertexArrays(1, VAOs);
    glGenBuffers(4, VBOs);
}

inline void genVAOs(int VAOidx) 
{
    glBindVertexArray(VAOs[VAOidx]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::VERTEX]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::NORM]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::UV]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[IVBO::INDICES]);
}

inline void loadData2VBOs(const vector<vec3> &vertexes, const vector<vec3> &norms, 
                const vector<vec2> &uvs, const vector<unsigned int> &indices) 
{
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertexes.size(), &vertexes[0], GL_STATIC_DRAW	);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::NORM]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * norms.size(), &norms[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[IVBO::UV]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);

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

inline void genObjects() 
{
    float distScale = 35.0 / 149597870.7;
	float radScale = 1.0 / 6378.1;
    
    SunCenter = vec3(0.0);
	SunRadius = pow(radScale * 696000.0, 0.5);
    genSphere(SunPoints, SunNorms, SunUVs, SunIndices, SunCenter, SunRadius, 96);
    Textures[TEX::SUN] = LoadTexture("resources/textures/sun.jpg");

    PlanetCenter = vec3(distScale * 149597890, 0.0, 0.0);
	PlanetRadius = pow(radScale * 6378.1, 0.5);
	genSphere(PlanetPoints, PlanetNorms, PlanetUVs, PlanetIndices, PlanetCenter, PlanetRadius, 72);
    Textures[TEX::PLANET] = LoadTexture("resources/textures/planet.jpg");  
}

inline void render(Shader &shader, mat4 model, IVAO vaoid, int elements, GLuint texture, GLuint textureUnit) 
{
    shader.use();
    glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAOs[vaoid]);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);
    glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, (void*)0);
}

inline void cleanup() 
{
    glDeleteVertexArrays(6, VAOs);
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
        glfwTerminate();
        exit(-1);
    }
    
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window ,resizeWindowCallback);
    glfwSetCursorPosCallback(window, mouseMovecallback);
    glfwSetScrollCallback(window, scrollcallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        errLog("Failed to init GLAD");
        glfwTerminate();
        exit(-1);
    }
    queryGLVersion();

    initOpenGL();
    Shader DefaultShader("resources/default.vert", "resources/default.frag");
    
    genObjects();

    initObjs();
    genVAOs(IVAO::DEFAULT);
    
    
    while (!glfwWindowShouldClose(window)) {
        frameHandler();
        keyPressHandler(window);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render------------------------------------------
        mat4 sunModel = mat4(1.0f);
        loadData2VBOs(SunPoints, SunNorms, SunUVs, SunIndices);
        render(DefaultShader, sunModel, IVAO::DEFAULT, SunIndices.size(), Textures[TEX::SUN], GL_TEXTURE0);
        
        mat4 planetModel = mat4(1.0);
        planetModel = translate(planetModel, vec3(5.0f, 0.0f, 0.0f));
        loadData2VBOs(PlanetPoints, PlanetNorms, PlanetUVs, PlanetIndices);
        render(DefaultShader, planetModel, IVAO::DEFAULT, PlanetIndices.size(), Textures[TEX::PLANET], GL_TEXTURE0);
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
}

void keyPressHandler(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}