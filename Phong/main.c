/**
 * @file main.c
 * @author Jovan Ivosevic
 * @brief Phong shading
 * @version 0.1
 * @date 2022-10-09
 *
 * Controls:
 * WASD - Movement
 * LEFT, RIGHT, UP and DOWN - Look around
 * 
 * 1 - Gouraud shading
 * 2 - Phong shading
 * 3 - Phong shading with Material
 * 4 - Phong shading with Material and Texture
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "cglm/cglm.h"
#include "cglm/cam.h"
#include "camera.h"
#include "model.h"
#include "texture.h"

 // NOTE(Jovan): Conditional compilation for cross platform sleep function
#ifdef _WIN32
 //  For Windows (32- and 64-bit)
#   include <windows.h>
#   define SLEEP(msecs) Sleep(msecs)
#elif __unix
 //  For linux, OSX, and other unixes
#   define _POSIX_C_SOURCE 199309L // or greater
#   include <time.h>
#   define SLEEP(msecs) do {            \
    struct timespec ts;             \
    ts.tv_sec = msecs/1000;         \
    ts.tv_nsec = msecs%1000*1000;   \
    nanosleep(&ts, NULL);           \
    } while (0)
#else
#   error "Unknown system"
#endif

int WindowWidth = 800;
int WindowHeight = 800;
const char* WindowTitle = "Phong";
const float TargetFPS = 60.0f;

/**
 * @brief Makes sure the passed value is inside [min, max] range
 *
 * @param x value to be clamped
 * @param min minimum value
 * @param max maximum value
 *
 * @return Clamped value
 */
static float
Clamp(float x, float min, float max) {
    return x < min ? min : x > max ? max : x;
}

/**
 * @brief Error callback function for GLFW. See GLFW docs for details
 *
 * @param error Error code
 * @param description Error message
 */
static void
ErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

/**
 * @brief Compiles GLSL shader
 *
 * @param type Type of shader: GL_VERTEX, GL_FRAGMENT
 * @param source Shader source code
 */
static unsigned CompileShader(GLenum type, const char* source);

/**
 * @brief Creates GLSL shader program
 *
 * @param vertexShaderSource Vertex shader source code
 * @param fragmentShaderSource Fragment shader source code
 */
static unsigned CreateShader(const char* vertexShaderSource, const char* fragmentShaderSource);

/**
 * @brief Sets mat4 uniform in GLSL shader
 *
 * @param programId GLSL Shader program ID
 * @param uniform Uniform name
 * @param m Pointer to matrix
 *
 */
static void SetUniform4m(unsigned programId, const char* uniform, const mat4* m);

/**
 * @brief Sets vec3 uniform in GLSL shader
 *
 * @param programId GLSL Shader program ID
 * @param uniform Uniform name
 * @param v Pointer to vec3
 *
 */
static void SetUniform3f(unsigned programId, const char* uniform, float x, float y, float z);

/**
 * @brief Sets float uniform in GLSL shader
 *
 * @param programId GLSL Shader program ID
 * @param uniform Uniform name
 * @param f float value
 *
 */
static void SetUniform1f(unsigned programId, const char* uniform, float f);

/**
 * @brief Sets integer uniform in GLSL shader
 *
 * @param programId GLSL Shader program ID
 * @param uniform Uniform name
 * @param i integer value
 *
 */
static void SetUniform1i(unsigned programId, const char* uniform, int i);

/**
 * @brief Draws flattened cubes
 *
 * @param vao - Cube VAO
 * @param shader - Shader
 */
static void DrawFloor(unsigned vao, unsigned shader, unsigned diffuse, unsigned specular);

int main() {
    GLFWwindow* Window = 0;
    if (!glfwInit()) {
        fprintf(stderr, "Failed to init glfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle, 0, 0);
    if (!Window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        fprintf(stderr, "Failed to init glew: %s\n", glewGetErrorString(GlewError));
        glfwTerminate();
        return -1;
    }

    glfwSetErrorCallback(ErrorCallback);

    glViewport(0, 0, WindowWidth, WindowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    unsigned CubeDiffuseTexture = LoadImageToTexture("res/container_diffuse.png");
    unsigned CubeSpecularTexture = LoadImageToTexture("res/container_specular.png");
    unsigned FloorDiffuseTexture= LoadImageToTexture("res/floor_diffuse.jpg");
    unsigned FloorSpecularTexture = LoadImageToTexture("res/floor_specular.jpg");
   /* unsigned pesakTexture = LoadImageToTexture("res/teksture/pesak.png");*/
    //unsigned pesakTexture = LoadImageToTexture("res/teksture/drvo.png");
    //unsigned pesakTexture = LoadImageToTexture("res/teksture/lisce.png");
    //unsigned pesakTexture = LoadImageToTexture("res/teksture/oblak.png");
    //unsigned pesakTexture = LoadImageToTexture("res/teksture/voda.png");
    

    float CubeVertices[] = {
        // X     Y     Z     NX    NY    NZ    U     V    FRONT SIDE
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // L U
                                                        // LEFT SIDE
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // RIGHT SIDE
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // BOTTOM SIDE
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // TOP SIDE
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // L D
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // R U
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // L U
                                                        // BACK SIDE
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // L D
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // R D
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // R U
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // L U
    };

    unsigned CubeVAO;
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);
    unsigned CubeVBO;
    glGenBuffers(1, &CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * 8 * sizeof(float), CubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Model Alduin;
    if (!LoadModel("res/alduin/alduin-dragon.obj", &Alduin)) {
        fprintf(stderr, "Failed to load alduin model\n");
        glfwTerminate();
        return -1;
    }

    Model Fox;
    if (!LoadModel("res/low-poly-fox/low-poly-fox.obj", &Fox)) {
        fprintf(stderr, "Failed to load low-poly-fox model\n");
        glfwTerminate();
        return -1;
    }

    unsigned GouraudShader = CreateShader("shaders/gouraud.vert", "shaders/basic.frag");
    glUseProgram(GouraudShader);
    SetUniform3f(GouraudShader, "uDirLight.Direction", 1.0f, -1.0f, 0.0f);
    SetUniform3f(GouraudShader, "uDirLight.Ka", 0.0f, 0.0f, 0.1f);
    SetUniform3f(GouraudShader, "uDirLight.Kd", 0.0f, 0.0f, 0.1f);
    SetUniform3f(GouraudShader, "uDirLight.Ks", 1.0f, 1.0f, 1.0f);

    SetUniform3f(GouraudShader, "uPointLight.Ka", 0.0f, 0.2f, 0.0f);
    SetUniform3f(GouraudShader, "uPointLight.Kd", 0.0f, 0.5f, 0.0f);
    SetUniform3f(GouraudShader, "uPointLight.Ks", 1.0f, 1.0f, 1.0f);
    // NOTE(Jovan): Parameters from https://wiki.ogre3d.org/-Point+Light+Attenuation
    SetUniform1f(GouraudShader, "uPointLight.Kc", 1.0f);
    SetUniform1f(GouraudShader, "uPointLight.Kl", 0.092f);
    SetUniform1f(GouraudShader, "uPointLight.Kq", 0.032f);

    SetUniform3f(GouraudShader, "uSpotlight.Position", 0.0f, 3.5f, -2.0f);
    SetUniform3f(GouraudShader, "uSpotlight.Direction",  0.0f, -1.0f, 1.0f);
    SetUniform3f(GouraudShader, "uSpotlight.Ka", 0.2f, 0.0f, 0.0f);
    SetUniform3f(GouraudShader, "uSpotlight.Kd", 0.5f, 0.0f, 0.0f);
    SetUniform3f(GouraudShader, "uSpotlight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(GouraudShader, "uSpotlight.Kc", 1.0f);
    SetUniform1f(GouraudShader, "uSpotlight.Kl", 0.092f);
    SetUniform1f(GouraudShader, "uSpotlight.Kq", 0.032f);

    SetUniform1f(GouraudShader, "uSpotlight.InnerCutOff", cos(glm_rad(12.5f)));
    SetUniform1f(GouraudShader, "uSpotlight.OuterCutOff", cos(glm_rad(17.5f)));
    glUseProgram(0);

    unsigned PhongShader = CreateShader("shaders/basic.vert", "shaders/phong.frag");
    glUseProgram(PhongShader);
    SetUniform3f(PhongShader, "uDirLight.Direction", 1.0f, -1.0f, 0.0f);
    SetUniform3f(PhongShader, "uDirLight.Ka", 0.0f, 0.0f, 0.1f);
    SetUniform3f(PhongShader, "uDirLight.Kd", 0.0f, 0.0f, 0.1f);
    SetUniform3f(PhongShader, "uDirLight.Ks", 1.0f, 1.0f, 1.0f);

    SetUniform3f(PhongShader, "uPointLight.Ka", 0.0f, 0.2f, 0.0f);
    SetUniform3f(PhongShader, "uPointLight.Kd", 0.0f, 0.5f, 0.0f);
    SetUniform3f(PhongShader, "uPointLight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongShader, "uPointLight.Kc", 1.0f);
    SetUniform1f(PhongShader, "uPointLight.Kl", 0.092f);
    SetUniform1f(PhongShader, "uPointLight.Kq", 0.032f);

    SetUniform3f(PhongShader, "uSpotlight.Position", 0.0f, 3.5f, -2.0f);
    SetUniform3f(PhongShader, "uSpotlight.Direction", 0.0f, -1.0f, 1.0f);
    SetUniform3f(PhongShader, "uSpotlight.Ka", 0.2f, 0.0f, 0.0f);
    SetUniform3f(PhongShader, "uSpotlight.Kd", 0.5f, 0.0f, 0.0f);
    SetUniform3f(PhongShader, "uSpotlight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongShader, "uSpotlight.Kc", 1.0f);
    SetUniform1f(PhongShader, "uSpotlight.Kl", 0.092f);
    SetUniform1f(PhongShader, "uSpotlight.Kq", 0.032f);

    SetUniform1f(PhongShader, "uSpotlight.InnerCutOff", cos(glm_rad(12.5f)));
    SetUniform1f(PhongShader, "uSpotlight.OuterCutOff", cos(glm_rad(17.5f)));
    glUseProgram(0);

    // NOTE(Jovan): Phong shader with material support
    unsigned PhongMaterialShader = CreateShader("shaders/basic.vert", "shaders/phong_material.frag");
    glUseProgram(PhongMaterialShader);
    SetUniform3f(PhongMaterialShader, "uDirLight.Direction", 1.0f, -1.0f, 0.0f);
    SetUniform3f(PhongMaterialShader, "uDirLight.Ka", 0.0f, 0.0f, 0.1f);
    SetUniform3f(PhongMaterialShader, "uDirLight.Kd", 0.0f, 0.0f, 0.1f);
    SetUniform3f(PhongMaterialShader, "uDirLight.Ks", 1.0f, 1.0f, 1.0f);

    SetUniform3f(PhongMaterialShader, "uPointLight.Ka", 0.0f, 0.2f, 0.0f);
    SetUniform3f(PhongMaterialShader, "uPointLight.Kd", 0.0f, 0.5f, 0.0f);
    SetUniform3f(PhongMaterialShader, "uPointLight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongMaterialShader, "uPointLight.Kc", 1.0f);
    SetUniform1f(PhongMaterialShader, "uPointLight.Kl", 0.092f);
    SetUniform1f(PhongMaterialShader, "uPointLight.Kq", 0.032f);

    SetUniform3f(PhongMaterialShader, "uSpotlight.Position", 0.0f, 3.5f, -2.0f);
    SetUniform3f(PhongMaterialShader, "uSpotlight.Direction", 0.0f, -1.0f, 1.0f);
    SetUniform3f(PhongMaterialShader, "uSpotlight.Ka", 0.2f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialShader, "uSpotlight.Kd", 0.5f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialShader, "uSpotlight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongMaterialShader, "uSpotlight.Kc", 1.0f);
    SetUniform1f(PhongMaterialShader, "uSpotlight.Kl", 0.092f);
    SetUniform1f(PhongMaterialShader, "uSpotlight.Kq", 0.032f);
    SetUniform1f(PhongMaterialShader, "uSpotlight.InnerCutOff", cos(glm_rad(12.5f)));
    SetUniform1f(PhongMaterialShader, "uSpotlight.OuterCutOff", cos(glm_rad(17.5f)));

    // NOTE(Jovan): Reflects the light's ambient component in full strength
    SetUniform3f(PhongMaterialShader, "uMaterial.Ka", 1.0f, 1.0f, 1.0f);
    // NOTE(Jovan): Diminishes the light's diffuse component by half, tinting it slightly red
    SetUniform3f(PhongMaterialShader, "uMaterial.Kd", 0.8f, 0.5f, 0.5f);
    // NOTE(Jovan): Makes the object really shiny
    SetUniform3f(PhongMaterialShader, "uMaterial.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongMaterialShader, "uMaterial.Shininess", 128.0f);
    glUseProgram(0);

    // NOTE(Jovan): Phong shader with material and texture support
    unsigned PhongMaterialTextureShader = CreateShader("shaders/basic.vert", "shaders/phong_material_texture.frag");
    glUseProgram(PhongMaterialTextureShader);
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Direction", 1.0f, -1.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Ka", 0.5f, 0.5f, 0.5f);
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Kd", 0.0f, 0.0f, 0.1f);
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Ks", 1.0f, 1.0f, 1.0f);

    SetUniform3f(PhongMaterialTextureShader, "uPointLight.Ka", 0.0f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uPointLight.Kd", 0.0f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uPointLight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongMaterialTextureShader, "uPointLight.Kc", 1.0f);
    SetUniform1f(PhongMaterialTextureShader, "uPointLight.Kl", 0.092f);
    SetUniform1f(PhongMaterialTextureShader, "uPointLight.Kq", 0.032f);

    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Position", 0.0f, 3.5f, -2.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Direction", 0.0f, -1.0f, 1.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Ka", 0.2f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Kd", 0.5f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Ks", 1.0f, 1.0f, 1.0f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.Kc", 1.0f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.Kl", 0.092f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.Kq", 0.032f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.InnerCutOff", cos(glm_rad(12.5f)));
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.OuterCutOff", cos(glm_rad(17.5f)));

    // NOTE(Jovan): Sets the first texture unit to be diffuse GL_TEXTURE0
    SetUniform1i(PhongMaterialTextureShader, "uMaterial.Kd", 0);
    // NOTE(Jovan): Sets the second texture unit to be specular GL_TEXTURE1
    SetUniform1i(PhongMaterialTextureShader, "uMaterial.Ks", 1);
    SetUniform1f(PhongMaterialTextureShader, "uMaterial.Shininess", 128.0f);
    glUseProgram(0);

    unsigned ColorShader = CreateShader("shaders/color.vert", "shaders/color.frag");

    mat4 Projection;
    glm_perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f, &Projection);

    Camera FPSCamera = CreateNewCamera();
    mat4 View;
    glm_lookat(FPSCamera.Position, FPSCamera.Target, FPSCamera.Up, &View);
    mat4 ModelMatrix;
    glm_mat4_identity(&ModelMatrix);

    float Vatra = 0.5f;
    int Upaljivost = 1;
    float StartTime = glfwGetTime();
    float EndTime = glfwGetTime();
    float TargetFrameTime = 1.0f / TargetFPS;
    float dt = EndTime - StartTime;
    // NOTE(Jovan): Current angle around Y axis, with regards to XZ plane at which the point light is situated at
    float Angle = 0.0f;
    // NOTE(Jovan): Distance of point light from center of rotation
    float Distance = 5.0f;
    unsigned CurrentShader = PhongMaterialTextureShader;
    glClearColor(0.2f, 0.2f, 1.0f, 1.0f);


    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();

        if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS) MoveCamera(&FPSCamera, -1.0f, 0.0f, dt);
        if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS) MoveCamera(&FPSCamera, 1.0f, 0.0f, dt);
        if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS) MoveCamera(&FPSCamera, 0.0f, -1.0f, dt);
        if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS) MoveCamera(&FPSCamera, 0.0f, 1.0f, dt);

        if (glfwGetKey(Window, GLFW_KEY_LEFT) == GLFW_PRESS) RotateCamera(&FPSCamera, -1.0f, 0.0f, dt);
        if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS) RotateCamera(&FPSCamera, 1.0f, 0.0f, dt);
        if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS) RotateCamera(&FPSCamera, 0.0f, -1.0f, dt);
        if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS) RotateCamera(&FPSCamera, 0.0f, 1.0f, dt);

        if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(Window, GLFW_TRUE);

        StartTime = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(CurrentShader);
        SetUniform4m(CurrentShader, "uProjection", &Projection);
        glm_lookat(FPSCamera.Position, FPSCamera.Target, FPSCamera.Up, &View);
        SetUniform4m(CurrentShader, "uView", &View);
        SetUniform3f(CurrentShader, "uViewPos", FPSCamera.Position[0], FPSCamera.Position[1], FPSCamera.Position[2]);

        // NOTE(Jovan): Rotate point light around 0, 0, -2
        Angle += dt;

        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube1  = { 5.0, 0.0, 0.0 };
        glm_translate(&ModelMatrix, &TranslateCube1);
        glm_scale(&ModelMatrix, (vec3) { 2.05f, 0.1f, 3.3f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, CubeDiffuseTexture);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // NOTE(Jovan): Floor render
        DrawFloor(CubeVAO, CurrentShader, FloorDiffuseTexture, FloorSpecularTexture);

        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateAlduin = { 0.0f, 1.0f, 4.0f };
        glm_translate(&ModelMatrix, &TranslateAlduin);
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        RenderModel(&Alduin);

        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateFox = { 4.0f, 1.0f, -4.0f };
        glm_translate(&ModelMatrix, &TranslateFox);
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        RenderModel(&Fox);

        // NOTE(Jovan): Draw point light
        if (Upaljivost == 1) {
            Vatra += 0.01;
            if (Vatra >= 1.0)
                Upaljivost = 0;
        }
        else {
            Vatra -= 0.01;
            if (Vatra <= 0.0)
                Upaljivost = 1;
        }
        vec3 PointLightPosition = { 0.0, 0.2, 0.0 };
        SetUniform3f(CurrentShader, "uPointLight.Position", PointLightPosition[0], PointLightPosition[1], PointLightPosition[2]);
        SetUniform3f(CurrentShader, "uPointLight.Ka", Vatra, 0.0, 0.0);
        SetUniform3f(CurrentShader, "uPointLight.Kd", Vatra, 0.0, 0.0);
        glUseProgram(ColorShader);
        SetUniform4m(ColorShader, "uProjection", &Projection);
        SetUniform4m(ColorShader, "uView", &View);

        glm_mat4_identity(&ModelMatrix);
        glm_translate(&ModelMatrix, &PointLightPosition);
        vec3 Scale = { 0.5f, 0.5f, 0.5f };
        glm_scale(ModelMatrix, Scale);
        SetUniform4m(ColorShader, "uModel", &ModelMatrix);
        glBindVertexArray(CubeVAO);
        SetUniform3f(ColorShader, "uCol", 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // NOTE(Jovan): Draw spotlight
        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateSpotlight = { 0.0f, 3.5f, -2.0f };
        glm_translate(&ModelMatrix, TranslateSpotlight);
        glm_scale(&ModelMatrix, Scale);
        SetUniform4m(ColorShader, "uModel", &ModelMatrix);
        SetUniform3f(ColorShader, "uCol", 1.0f, 0.0f, 0.0f);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(Window);
        // NOTE(Jovan): Time management for stable framerates
        EndTime = glfwGetTime();
        float WorkTime = EndTime - StartTime;
        if (WorkTime < TargetFrameTime) {
            int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
            SLEEP(DeltaMS);
            EndTime = glfwGetTime();
        }
        dt = EndTime - StartTime;
    }
    FreeModelResources(&Alduin);
    FreeModelResources(&Fox);

    glfwTerminate();
    return 0;
}


unsigned CompileShader(GLenum type, const char* source) {
    unsigned int id, result, logLength;
    char* sourceCode = NULL, * errorMessage = NULL;
    long shaderSize;
    FILE* inputFile = 0;

    id = glCreateShader(type);
    inputFile = fopen(source, "r");
    if (!inputFile)
    {
        fprintf(stderr, "ERROR: %s file not found.\n", source);
        return 0;
    }

    fseek(inputFile, 0L, SEEK_END);
    shaderSize = ftell(inputFile);
    fseek(inputFile, 0L, SEEK_SET);
    sourceCode = (char*)calloc(shaderSize, sizeof(char));

    if (!sourceCode) {
        fprintf(stderr, "ERROR: Could not load source code.\n");
        return 0;
    }

    fread(sourceCode, sizeof(char), shaderSize, inputFile);
    fclose(inputFile);

    glShaderSource(id, 1, &sourceCode, NULL);
    glCompileShader(id);

    free(sourceCode);

    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
        errorMessage = (char*)calloc(logLength, sizeof(char));
        glGetShaderInfoLog(id, logLength, &logLength, errorMessage);
        fprintf(stderr, "ERROR compiling shader (%s): %s\n", source, errorMessage);
        free(errorMessage);
    }

    return id;
}

unsigned CreateShader(const char* vertexShaderSource, const char* fragmentShaderSource) {
    unsigned int program, vertexShader, fragmentShader;

    program = glCreateProgram();
    vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

static void SetUniform4m(unsigned programId, const char* uniform, const mat4* m) {
    glUniformMatrix4fv(glGetUniformLocation(programId, uniform), 1, GL_FALSE, (float*)m);
}

static void SetUniform3f(unsigned programId, const char* uniform, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(programId, uniform), x, y, z);
}

static void SetUniform1f(unsigned programId, const char* uniform, float f) {
    glUniform1f(glGetUniformLocation(programId, uniform), f);
}

static void SetUniform1i(unsigned programId, const char* uniform, int i) {
    glUniform1i(glGetUniformLocation(programId, uniform), i);
}

static void DrawFloor(unsigned vao, unsigned shader, unsigned diffuse, unsigned specular) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular);

    glBindVertexArray(vao);

    float Size = 4.0f;
    for (int i = -2; i < 4; ++i) {
        for (int j = -2; j < 4; ++j) {
            mat4 Model;
            glm_mat4_identity(&Model);
            vec3 Translate = { i * Size, -2.0f, j * Size };
            glm_translate(&Model, &Translate);
            vec3 Scale = { Size, 0.1f, Size };
            glm_scale(&Model, &Scale);
            SetUniform4m(shader, "uModel", &Model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);
}