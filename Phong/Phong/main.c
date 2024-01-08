#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include "cglm/cglm.h"
#include "cglm/cam.h"
#include "camera.h"
#include "model.h"
#include "texture.h"

#ifdef _WIN32
//  For Windows (32- and 64-bit)
#   include <windows.h>
#   define SLEEP(msecs) Sleep(msecs)
#endif

int WindowWidth = 900;
int WindowHeight = 900;
const char* WindowTitle = "Phong";
const float TargetFPS = 60.0f;

static float
Clamp(float x, float min, float max) {
    return x < min ? min : x > max ? max : x;
}

static unsigned CompileShader(GLenum type, const char* source);
static unsigned CreateShader(const char* vertexShaderSource, const char* fragmentShaderSource);
static void SetUniform4m(unsigned programId, const char* uniform, const mat4* m);
static void SetUniform3f(unsigned programId, const char* uniform, float x, float y, float z);
static void SetUniform1f(unsigned programId, const char* uniform, float f);
static void SetUniform1i(unsigned programId, const char* uniform, int i);
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

    glViewport(0, 0, WindowWidth, WindowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    unsigned vodaTexture = LoadImageToTexture("res/map.png");
    unsigned oblakTexture = LoadImageToTexture("res/teksture/oblak.png");
    unsigned jelenTexture = LoadImageToTexture("res/teksture/jelen.jpg");



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

    Model Jelen;
    if (!LoadModel("res/Jelen/jelen.obj", &Jelen)) {
        fprintf(stderr, "Failed to load alduin model\n");
        glfwTerminate();
        return -1;
    }


    unsigned PhongMaterialTextureShader = CreateShader("shaders/basic.vert", "shaders/phong_material_texture.frag");
    glUseProgram(PhongMaterialTextureShader);
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Direction", -3.0, -3.0, 0.0); //vektor pravca sunca
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Ka", 0.0f, 0.0f, 0.0f);  //kombinujemo da bismo dobili zuckastu boju
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Kd", 1.0f, 1.0f, 1.0f); //ka kd i ks su intenzitet svetla
    SetUniform3f(PhongMaterialTextureShader, "uDirLight.Ks", 1.0f, 1.0f, 1.0f);

    SetUniform3f(PhongMaterialTextureShader, "uPointLight.Ka", 0.1f, 0.1f, 0.0f); //ovo je boja vatrice
    SetUniform3f(PhongMaterialTextureShader, "uPointLight.Kd", 0.8f, 0.5f, 0.0f);  // kc kl i kq su slabljenja sa razdaljinom
    SetUniform3f(PhongMaterialTextureShader, "uPointLight.Ks", 1.0f, 1.0f, 0.0f);
    SetUniform1f(PhongMaterialTextureShader, "uPointLight.Kc", 1.0f);
    SetUniform1f(PhongMaterialTextureShader, "uPointLight.Kl", 0.8f);
    SetUniform1f(PhongMaterialTextureShader, "uPointLight.Kq", 2.0f);

    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Position", 3.0f, 2.0f, -2.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Direction", 0.0f, -1.0f, 1.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Ka", 0.5f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Kd", 0.5f, 0.0f, 0.0f);
    SetUniform3f(PhongMaterialTextureShader, "uSpotlight.Ks", 0.5f, 0.0f, 0.0f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.Kc", 1.0f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.Kl", 0.092f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.Kq", 0.032f);
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.InnerCutOff", cos(glm_rad(5.5f)));
    SetUniform1f(PhongMaterialTextureShader, "uSpotlight.OuterCutOff", cos(glm_rad(17.5f)));

    SetUniform1i(PhongMaterialTextureShader, "uMaterial.Kd", 0);  // kanal za Difuzne teksture
    SetUniform1i(PhongMaterialTextureShader, "uMaterial.Ks", 1);  // kanal za Spekularne teksture
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

    int StatusOblaci = 1;
    float Vatra = 0.5f;
    int Upaljivost = 1;
    float NivoMora = 0.3;
    int MoreFaza = 1;
    float StartTime = glfwGetTime();
    float EndTime = glfwGetTime();
    float TargetFrameTime = 1.0f / TargetFPS;
    float dt = EndTime - StartTime;
    // Current angle around Y axis, with regards to XZ plane at which the point light is situated at
    float Angle = 0.0f;
    // Distance of point light from center of rotation
    float Distance = 5.0f;
    unsigned CurrentShader = PhongMaterialTextureShader;
    glClearColor(0.2f, 0.2f, 0.6f, 1.0f);


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


        int ZKeyState = glfwGetKey(Window, GLFW_KEY_Z);
        int XKeyState = glfwGetKey(Window, GLFW_KEY_X);

        // Rotate point light around 0, 0, -2
        Angle += dt;


        glm_mat4_identity(&ModelMatrix);
        vec3 Voda = { -3.0, NivoMora - 0.4, -4 };
        glm_translate(&ModelMatrix, &Voda);
        glm_scale(&ModelMatrix, (vec3) { 25.f, 1.f, 25.f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, vodaTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 1.0);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 24, 6);


        //oblak
        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube6 = { -8., 8.6, -1.5 };
        glm_translate(&ModelMatrix, &TranslateCube6);
        glm_scale(&ModelMatrix, (vec3) { 4 * 1.5f, 2 * 0.7f, 4 * 1.9f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oblakTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.5);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube7 = { -8.0, 9.6, -1.2 };
        glm_translate(&ModelMatrix, &TranslateCube7);
        glm_scale(&ModelMatrix, (vec3) { 4 * 1.2f, 2 * 0.5f, 4 * 1.6f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oblakTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.5);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube8 = { 2., 8.6, -10.5 };
        glm_translate(&ModelMatrix, &TranslateCube8);
        glm_scale(&ModelMatrix, (vec3) { 3 * 2.f, 2 * 0.7f, 3 * 1.7f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oblakTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.5);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube9 = { 2.0, 9.6, -10.2 };
        glm_translate(&ModelMatrix, &TranslateCube9);
        glm_scale(&ModelMatrix, (vec3) { 3 * 1.8f, 2 * 0.5f, 3 * 1.4f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oblakTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.5);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube10 = { 2., 8.6, 2.5 };
        glm_translate(&ModelMatrix, &TranslateCube8);
        glm_scale(&ModelMatrix, (vec3) { 3 * 2.f, 2 * 0.7f, 3 * 1.7f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oblakTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.5);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glm_mat4_identity(&ModelMatrix);
        vec3 TranslateCube11 = { 2.0, 9.6, 2.2 };
        glm_translate(&ModelMatrix, &TranslateCube11);
        glm_scale(&ModelMatrix, (vec3) { 5 * 1.8f, 2 * 0.5f, 5 * 1.4f });
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oblakTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.5);
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);



        // jelen
        glm_mat4_identity(&ModelMatrix);
        vec3 Translatejelena = { -3.0, 1, -6.0 };
        glm_scale(&ModelMatrix, (vec3) { 0.3f, 0.3f, 0.3f });
        glm_translate(&ModelMatrix, &Translatejelena);
        SetUniform4m(CurrentShader, "uModel", &ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, jelenTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0.0);
        RenderModel(&Jelen);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);


        glfwSwapBuffers(Window);
        EndTime = glfwGetTime();
        float WorkTime = EndTime - StartTime;
        if (WorkTime < TargetFrameTime) {
            int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
            SLEEP(DeltaMS);
            EndTime = glfwGetTime();
        }
        dt = EndTime - StartTime;
    }
    FreeModelResources(&Jelen);


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