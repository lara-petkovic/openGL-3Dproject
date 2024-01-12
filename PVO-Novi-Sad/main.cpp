// Autor: Lara Petkovi� RA 185/2020
// Opis: Protiv-vazdu�na odbrana Novog Sada 

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#define CRES 30
#define DRONES_LEFT 7
#define PI 3.141592
#define CAMERA_X_LOC 0.0f   //0.0f
#define CAMERA_Y_LOC 0.6f   //0.6f
#define CAMERA_Z_LOC -1.0f  //-1.0f

#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Za koptere
#include <ctime>
#include <random>
#include <chrono>

using namespace glm;
using namespace std;

struct ModelData {
    vector<vec3> vertices;
    vector<vec2> textureCoords;
    vector<vec3> normals;
};


unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
void setXZCircle(float  circle[96], float r, float xPomeraj, float zPomeraj);
void setXYCircle(float  circle[96], float r, float xPomeraj, float zPomeraj);
static unsigned loadImageToTexture(const char* filePath);
void moveDrone(GLFWwindow* window, float& droneX, float& droneY, float droneSpeed, unsigned int wWidth, unsigned int wHeight);
void generateHelicopterPositions(int number);
void moveHelicoptersTowardsCityCenter(float cityCenterX, float cityCenterY, float speed);
bool checkCollision(float object1X, float object1Y, float object1Radius, float object2X, float object2Y, float object2Radius);
bool isDroneOutsideScreen(float droneX, float droneY);
ModelData loadModel(const char* filePath);
void processMesh(aiMesh* mesh, const aiScene* scene, ModelData& modelData);
void processNode(aiNode* node, const aiScene* scene, ModelData& modelData);


struct Location {
    float x;
    float y;
};

float droneX = 0.0f;
float droneY = 0.0f;
float droneZ = -0.45f;
float droneSpeed = 0.0002f;
bool isSpacePressed = false;
bool wasSpacePressed = false;
bool coptersOnScreen = true;
int numberOfCollied = 0;
bool isMapHidden = false;
Location helicopterPositions[5];
auto startTime = chrono::high_resolution_clock::now();


int main(void)
{
    if (!glfwInit())
    {
        cout << "Greska pri ucitavanju GLFW biblioteke!\n";
        return 1;
    }

    generateHelicopterPositions(5);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 900;
    unsigned int wHeight = 900;
    const char wTitle[] = "Protiv-vazdusna odbrana Novog Sada";
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int xPos = (mode->width - wWidth) / 2;
    int yPos = (mode->height - wHeight) / 2;
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    glfwSetWindowPos(window, xPos, yPos);

    if (window == NULL)
    {
        cout << "Greska pri formiranju prozora!\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK)
    {
        cout << "Greska pri ucitavanju GLEW biblioteke!\n";
        return 3;
    }

    unsigned int textureShader = createShader("texture.vert", "texture.frag");
    unsigned int baseShader = createShader("base.vert", "base.frag");
    unsigned int dronShader = createShader("dron.vert", "dron.frag");
    int colorLoc = glGetUniformLocation(textureShader, "color");

    float vertices[] = {
   // X     Y      Z       S    T  
    -1.0, -0.01, -1.0,    0.0, 0.0,    // Stavila sam Z osu na -0.01 radi testiranja dubine -> mapa je malo niza od svih ostalih objekata
     1.0, -0.01, -1.0,    1.0, 0.0,
    -1.0, -0.01,  1.0,    0.0, 1.0,

     1.0, -0.01, -1.0,    1.0, 0.0,
     1.0, -0.01,  1.0,    1.0, 1.0
    };

    // *********************************************************************** MODELI ***********************************************************************
    // 
    // Planina ----------------------------------------------------------------------------------------------------------
    const char* modelPath1 = "res/mountain/Mountain.obj";
    ModelData mountain = loadModel(modelPath1);

    unsigned int mountainVAO, mountainVBO;
    glGenVertexArrays(1, &mountainVAO);
    glGenBuffers(1, &mountainVBO);
    glBindVertexArray(mountainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mountainVBO);
    glBufferData(GL_ARRAY_BUFFER, mountain.vertices.size() * sizeof(vec3), &mountain.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Dron ------------------------------------------------------------------------------------------------------------
    const char* modelPath2 = "res/drone/Drone.obj";
    ModelData drone = loadModel(modelPath2);

    unsigned int droneVAO, droneVBO;
    glGenVertexArrays(1, &droneVAO);
    glGenBuffers(1, &droneVBO);
    glBindVertexArray(droneVAO);
    glBindBuffer(GL_ARRAY_BUFFER, droneVBO);
    glBufferData(GL_ARRAY_BUFFER, drone.vertices.size() * sizeof(vec3), &drone.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Oblak1 ------------------------------------------------------------------------------------------------------------
    const char* modelPath3 = "res/clouds/Cloud.obj";
    ModelData cloud1 = loadModel(modelPath3);

    unsigned int cloud1VAO, cloud1VBO;
    glGenVertexArrays(1, &cloud1VAO);
    glGenBuffers(1, &cloud1VBO);
    glBindVertexArray(cloud1VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cloud1VBO);
    glBufferData(GL_ARRAY_BUFFER, cloud1.vertices.size() * sizeof(vec3), &cloud1.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ******************************************************************************************************************************************************

    glUseProgram(0);



    unsigned int stride = (3 + 2) * sizeof(float);

    unsigned int VAO[3];
    glGenVertexArrays(3, VAO);
    unsigned int VBO[3];
    glGenBuffers(3, VBO);

    // VAO i VBO teksture -------------------------------------------------------------    
    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);
    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Renderovanje teksture -----------------------------------------------------------
    unsigned mapTexture = loadImageToTexture("res/novi-sad.png");
    unsigned cloudTexture = loadImageToTexture("res/novi-sad.png");

    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc = glGetUniformLocation(textureShader, "uTex");
    glUniform1i(uTexLoc, 0);

    // Opis baze -----------------------------------------------------------------------
    float baseCircle[CRES * 3 + 6];
    setXZCircle(baseCircle, 0.07, 0.0, -0.45);

    // VAO i VBO baze
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(baseCircle), baseCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    // Opis centra Novog Sada ----------------------------------------------------------
    float cityCenterCircle[CRES * 3 + 6];
    setXZCircle(cityCenterCircle, 0.017, 0.42, 0.08);

    // VAO i VBO centra Novog Sada
    glGenVertexArrays(1, &VAO[2]);
    glGenBuffers(1, &VBO[2]);
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cityCenterCircle), cityCenterCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Opis drona ----------------------------------------------------------------------
    float blueCircle[CRES * 3 + 6];
    setXZCircle(blueCircle, 0.03, 0.0, 0.0);

    // VAO i VBO drona
    unsigned int VAOBlue, VBOBlue;
    glGenVertexArrays(1, &VAOBlue);
    glGenBuffers(1, &VBOBlue);
    glBindVertexArray(VAOBlue);
    glBindBuffer(GL_ARRAY_BUFFER, VBOBlue);
    glBufferData(GL_ARRAY_BUFFER, sizeof(blueCircle), blueCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // LED sijalica pozadina -> indikator da li je letelica u vazduhu -------------------
    float LEDBackgroundCircle[CRES * 3 + 6];
    setXYCircle(LEDBackgroundCircle, 0.045, -0.70, 0.85);

    // VAO i VBO LED-a
    unsigned int VAOLEDBackground, VBOLEDBackground;
    glGenVertexArrays(1, &VAOLEDBackground);
    glGenBuffers(1, &VBOLEDBackground);
    glBindVertexArray(VAOLEDBackground);
    glBindBuffer(GL_ARRAY_BUFFER, VBOLEDBackground);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LEDBackgroundCircle), LEDBackgroundCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // LED sijalica -> indikator da li je letelica u vazduhu ----------------------------
    float LEDCircle[CRES * 3 + 6];
    setXYCircle(LEDCircle, 0.02, -0.70, 0.85);

    // VAO i VBO LED-a
    unsigned int VAOLED, VBOLED;
    glGenVertexArrays(1, &VAOLED);
    glGenBuffers(1, &VBOLED);
    glBindVertexArray(VAOLED);
    glBindBuffer(GL_ARRAY_BUFFER, VBOLED);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LEDCircle), LEDCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    int dronesLeft = DRONES_LEFT;

    // VAO i VBO preostalih dronova -----------------------------------------------------
    unsigned int VAOdronLeft[DRONES_LEFT];
    unsigned int VBOdronLeft[DRONES_LEFT];
    float dronLeftCircle[CRES * 3 + 6];
    for (int i = 0; i < dronesLeft; ++i) {

        setXYCircle(dronLeftCircle, 0.02, 0.7 + 0.04 * i, -0.8);

        glGenVertexArrays(1, &VAOdronLeft[i]);
        glGenBuffers(1, &VBOdronLeft[i]);
        glBindVertexArray(VAOdronLeft[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOdronLeft[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(dronLeftCircle), dronLeftCircle, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }


    // VAO i VBO svetla ------------------------------------------------------------------
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    mat4 model = mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    unsigned int modelLocTex = glGetUniformLocation(textureShader, "uM");
    unsigned int modelLocDron = glGetUniformLocation(dronShader, "uM");
    unsigned int modelLocBase = glGetUniformLocation(baseShader, "uM");


    mat4 view; //Matrica pogleda (kamere)
    view = lookAt(vec3(CAMERA_X_LOC, CAMERA_Y_LOC, CAMERA_Z_LOC), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    unsigned int viewLocTex = glGetUniformLocation(textureShader, "uV");
    unsigned int viewLocDron = glGetUniformLocation(dronShader, "uV");
    unsigned int viewLocBase = glGetUniformLocation(baseShader, "uV");


    mat4 projection = perspective(radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    unsigned int projectionLocTex = glGetUniformLocation(textureShader, "uP");
    unsigned int projectionLocDron = glGetUniformLocation(dronShader, "uP");
    unsigned int projectionLocBase = glGetUniformLocation(baseShader, "uP");

    bool wasXpressed = false;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            isMapHidden = true;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            isMapHidden = false;
        }

        if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && !wasXpressed && dronesLeft > 0) || isDroneOutsideScreen(droneX, droneZ) || droneY < 0.0f) {
            wasXpressed = true;
            if (!isSpacePressed) {
                wasSpacePressed = !wasSpacePressed;
            }
            isSpacePressed = true;
            droneX = 0.0f;
            droneY = 0.0f;
            droneZ = -0.45f;
            dronesLeft--;
        }
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE) {
            wasXpressed = false;
        }


        glClearColor(0.1, 0.1, 0.10023082, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        model = mat4(1.0);
        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));

        glUseProgram(textureShader);
        model[0] *= -1;
        glUniformMatrix4fv(modelLocTex, 1, GL_FALSE, value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
        glUniformMatrix4fv(viewLocTex, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projectionLocTex, 1, GL_FALSE, value_ptr(projection));
        glBindVertexArray(VAO[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mapTexture);

        if (!isMapHidden)
        {
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        // Renderovanje baze
        glUseProgram(baseShader);
        glBindVertexArray(VAO[1]);
        colorLoc = glGetUniformLocation(baseShader, "color");
        glUniform3f(colorLoc, 0.0, 1.0, 0.0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(baseCircle) / (3 * sizeof(float)));


        // Renderovanje preostalih dronova    0, 1, -1 ----------------------------------------------------------
        glCullFace(GL_FRONT);

        for (int i = 0; i < dronesLeft; ++i) {
            glBindVertexArray(VAOdronLeft[i]);
            mat4 model = mat4(1.0f);
            model = translate(model, vec3(-0.95f, 1.15f, 0.0f));
            model = rotate(model, 0.8f, vec3(1.0f, 0.0f, 0.0f));
            model = scale(model, vec3(0.8f, 0.8f, 0.8f));
            glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));
            glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(dronLeftCircle) / (3 * sizeof(float)));
        }
        glCullFace(GL_BACK);

        // Renderovanje pozadine LED sijalice -------------------------------------------------------------------
        glUseProgram(baseShader);
        glBindVertexArray(VAOLEDBackground);

        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model)); // (Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
        glUniformMatrix4fv(viewLocBase, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projectionLocBase, 1, GL_FALSE, value_ptr(projection));
        
        colorLoc = glGetUniformLocation(baseShader, "color");
        glUniform3f(colorLoc, 0.3, 0.2, 0.2);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(LEDBackgroundCircle) / (3 * sizeof(float)));

        // Renderovanje LED sijalice -> upaljena ako postoji letelica u vazduhu
        glBindVertexArray(VAOLED);
        colorLoc = glGetUniformLocation(baseShader, "color");
        if (coptersOnScreen) {
            glUniform3f(colorLoc, 1.0, 0.0, 0.0); // Crvena boja LED sijalice kada ima helikoptera
        }
        else {
            glUniform3f(colorLoc, 0.0, 1.0, 0.0); // Zelena boja LED sijalice kada nema helikoptera
        }
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(LEDCircle) / (3 * sizeof(float)));



        // Renderovanje centra Novog Sada ------------------------------------------------------------------------
        glBindVertexArray(VAO[2]);
        glUniform3f(colorLoc, 0.0, 0.0, 0.0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(cityCenterCircle) / (3 * sizeof(float)));


        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!isSpacePressed) {
                wasSpacePressed = !wasSpacePressed;
            }
            isSpacePressed = true;
            droneX = 0.0f;
            droneY = 0.0f;
            droneZ = -0.45f;
        }
        else {
            isSpacePressed = false;
        }

        if (wasSpacePressed && dronesLeft > 0)
        {
            moveDrone(window, droneX, droneZ, droneSpeed, wWidth, wHeight);

            // Renderovanje 2D drona
            glUseProgram(dronShader);
            glUniformMatrix4fv(modelLocDron, 1, GL_FALSE, value_ptr(model));
            glUniformMatrix4fv(viewLocDron, 1, GL_FALSE, value_ptr(view));
            glUniformMatrix4fv(projectionLocDron, 1, GL_FALSE, value_ptr(projection));
            glBindVertexArray(VAOBlue);
            GLint translationLoc = glGetUniformLocation(dronShader, "uTranslation");
            glUniform2f(translationLoc, droneX, droneZ);
            colorLoc = glGetUniformLocation(dronShader, "color");
            glUniform3f(colorLoc, 0.0, 0.0, 1.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(blueCircle) / (3 * sizeof(float)));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                droneY += droneSpeed;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                droneY -= droneSpeed;
                if (droneY == 0.0f) {

                }
            }

            // Renderovanje 3D drona
            glBindVertexArray(droneVAO);
            mat4 model3D = mat4(1.0f);
            model3D = translate(model3D, vec3(-droneX, droneY, droneZ));
            model3D = scale(model3D, vec3(0.15f));
            glUniform3f(colorLoc, 0.0 / 255.0, 200.0 / 255.0, 35.0 / 255.0);
            glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model3D));
            GLint translationLocDrone = glGetUniformLocation(dronShader, "uTranslation");
            glUniform3f(translationLocDrone, 0.0f, -0.4f, -0.43f);
            glDrawArrays(GL_TRIANGLES, 0, drone.vertices.size());
            glBindVertexArray(0);
        }


        // Proteklo vreme od pocetka programa
        auto currentTime = chrono::high_resolution_clock::now();
        float elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;


        // Renderovanje helikoptera -------------------------------------------------------------------------
        for (int i = 0; i < 5; i++) {
            //// Izra�unamo vektor od helikoptera do centra
            //float dirX = 0.42 - helicopterPositions[i].x;
            //float dirY = 0.08 - helicopterPositions[i].y;

            //// Izra�unamo razdaljinu od koptera do centra - Pitagora
            //float distance = sqrt(dirX * dirX + dirY * dirY);

            //// Normalizujemo vektor
            //dirX /= distance;
            //dirY /= distance;

            //// Prilagodimo brzinu pulsiranja na osnovu udaljenosti od centra - osnovna brzina je 5.0f
            //float pulseSpeed = 5.0f + 10.0f * (1.0f - distance);

            //// Izra�unamo faktor pulsiranja na osnovu vremena i udaljenosti
            //float pulseFactor = 0.5f + 0.5f * sin(elapsedTime * pulseSpeed);
            //float redIntensity = 1.0;
            //float greenIntensity = 1.0 - pulseFactor;
            //float blueIntensity = 1.0 - pulseFactor;

            float redIntensity = 0.28;
            float greenIntensity = 0.3;
            float blueIntensity = 0.2001;

            glUseProgram(dronShader);
            glUniformMatrix4fv(modelLocDron, 1, GL_FALSE, value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
            glUniformMatrix4fv(viewLocDron, 1, GL_FALSE, value_ptr(view));
            glUniformMatrix4fv(projectionLocDron, 1, GL_FALSE, value_ptr(projection));
            glBindVertexArray(VAOBlue);
            GLint translationLoc = glGetUniformLocation(dronShader, "uTranslation");
            glUniform2f(translationLoc, helicopterPositions[i].x, helicopterPositions[i].y);
            colorLoc = glGetUniformLocation(dronShader, "color");
            glUniform3f(colorLoc, redIntensity, greenIntensity, blueIntensity);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(blueCircle) / (3 * sizeof(float)));
            glBindVertexArray(0);

            if (checkCollision(droneX, droneZ, 0.03, helicopterPositions[i].x, helicopterPositions[i].y, 0.03)) {
                droneX = 0.0f; // Resetovanje pozicije drona
                droneY = 0.0f;
                droneZ = -0.45f;
                helicopterPositions[i].x = 1000.0f; // Skloni helikopter sa scene
                helicopterPositions[i].y = 1000.0f;
                numberOfCollied++;
                wasSpacePressed = false;
                dronesLeft--;
            }
            if (numberOfCollied == 5) {
                coptersOnScreen = false;
            }
        }

        moveHelicoptersTowardsCityCenter(0.42, 0.08, droneSpeed / 3);

        // Renderovanje planine ------------------------------------------------------------------------------
        glUseProgram(textureShader);
        glBindVertexArray(mountainVAO);

        // Uniforme teksture planine
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mapTexture);
        glUniform1i(glGetUniformLocation(textureShader, "uTex"), 0);

        model = scale(model, vec3(0.1, 0.1, 0.001));
        model = translate(model, vec3(0.0, 0.0, -0.3));
        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));

        glDrawArrays(GL_TRIANGLES, 0, mountain.vertices.size());
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);

        // Renderovanje seta oblaka ------------------------------------------------------------------------------
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(baseShader);
        glBindVertexArray(cloud1VAO);

        // Renderovanje prednjeg oblaka seta
        colorLoc = glGetUniformLocation(baseShader, "color");
        glUniform3f(colorLoc, 0.7, 0.7, 0.7);
        GLuint alphaLoc = glGetUniformLocation(baseShader, "uAlpha");
        glUniform1f(alphaLoc, 0.5);
        mat4 model1 = mat4(1.0f);
        model1 = scale(model1, vec3(0.1));
        model1 = translate(model1, vec3(0.0, 9.0, 7.0));
        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model1));

        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, cloud1.vertices.size());

        // Renderovanje zadnjeg oblaka seta
        colorLoc = glGetUniformLocation(baseShader, "color");
        glUniform3f(colorLoc, 0.7, 0.7, 0.7);

        glUniform1f(alphaLoc, 0.5);
        mat4 model2 = mat4(1.0f);
        model2 = scale(model2, vec3(0.1));
        model2 = translate(model2, vec3(0.0, 9.8, 9.0));
        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model2));

        glDrawArrays(GL_TRIANGLES, 0, cloud1.vertices.size());
        glBindVertexArray(0);
        glDisable(GL_BLEND);
        glUniform1f(alphaLoc, 0.0);

        glEnable(GL_CULL_FACE);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &mapTexture);
    glDeleteTextures(2, &cloudTexture);
    glDeleteBuffers(3, VBO);
    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(1, &VBOBlue);
    glDeleteVertexArrays(1, &VAOBlue);
    glDeleteBuffers(1, &VBOLED);
    glDeleteVertexArrays(1, &VAOLED);
    glDeleteBuffers(1, &VBOLEDBackground);
    glDeleteVertexArrays(1, &VAOLEDBackground);
    glDeleteBuffers(1, &mountainVBO);
    glDeleteVertexArrays(1, &mountainVAO);
    glDeleteBuffers(1, &droneVBO);
    glDeleteVertexArrays(1, &droneVAO);
    glDeleteBuffers(1, &cloud1VBO);
    glDeleteVertexArrays(1, &cloud1VAO);

    glDeleteProgram(textureShader);
    glDeleteProgram(baseShader);

    for (int i = 0; i < DRONES_LEFT; i++) {
        glDeleteVertexArrays(1, &VAOdronLeft[i]);
        glDeleteBuffers(1, &VBOdronLeft[i]);
    }

    glfwTerminate();
    return 0;
}

bool checkCollision(float object1X, float object1Y, float object1Radius, float object2X, float object2Y, float object2Radius) {
    float distance = sqrt(pow(object2X - object1X, 2) + pow(object2Y - object1Y, 2));
    return distance < (object1Radius + object2Radius);
}

void moveHelicoptersTowardsCityCenter(float cityCenterX, float cityCenterY, float speed) {
    for (int i = 0; i < 5; i++) {
        // Izracunamo vektor od helikoptera do centra
        float dirX = cityCenterX - helicopterPositions[i].x;
        float dirY = cityCenterY - helicopterPositions[i].y;

        // Izracunamo razdaljinu od koptera do centra
        float distance = sqrt(dirX * dirX + dirY * dirY);

        // Normalizujemo vektor
        dirX /= distance;
        dirY /= distance;

        // Pomeramo helikopter ka centru odredjenom brzinom
        helicopterPositions[i].x += dirX * speed;
        helicopterPositions[i].y += dirY * speed;
    }
}

void generateHelicopterPositions(int number) {
    srand(static_cast<unsigned>(time(nullptr)));

    for (int i = 0; i < number; ++i) {
        int strana = rand() % 4;
        if (strana == 0) {                                    // Desna stranica
            helicopterPositions[i].x = 1;
            string randomFloat = "0.";
            randomFloat.append(to_string(rand() % 10));
            randomFloat.append(to_string(rand() % 10));
            helicopterPositions[i].y = stof(randomFloat);
        }
        else if (strana == 1) {                               // Gornja stranica
            string randomFloat = "0.";
            randomFloat.append(to_string(rand() % 10));
            randomFloat.append(to_string(rand() % 10));
            helicopterPositions[i].x = stof(randomFloat);
            helicopterPositions[i].y = 1;
        }
        //else if (strana == 2) {                               // Leva stranica
        else{
            helicopterPositions[i].x = -1;
            string randomFloat = "0.";
            randomFloat.append(to_string(rand() % 10));
            randomFloat.append(to_string(rand() % 10));
            helicopterPositions[i].y = stof(randomFloat);
        }
        //else {
        //    string randomFloat = "0.";                        // Donja stranica -> Ne moze nam dron doci sa nase planine
        //    randomFloat.append(to_string(rand() % 10));
        //    randomFloat.append(to_string(rand() % 10));
        //    helicopterPositions[i].x = stof(randomFloat);
        //    helicopterPositions[i].y = -1;
        //}
    }
}

bool isDroneOutsideScreen(float droneX, float droneY)
{
    return (droneX < -1.0f || droneX > 1.0f || droneY < -1.0f || droneY > 1.0f);
}

void moveDrone(GLFWwindow* window, float& droneX, float& droneY, float droneSpeed, unsigned int wWidth, unsigned int wHeight)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        droneY += droneSpeed;
        // Bilo nekad: droneY = fmax(-1.0f, fmin(droneY + droneSpeed, 1.0f)); itd.
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        droneY -= droneSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        droneX -= droneSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        droneX += droneSpeed;
    }
}

void setXZCircle(float  circle[96], float r, float xPomeraj, float zPomeraj)
{
    float centerX = 0.0;
    float centerY = 0.0;
    float centerZ = 0.0;

    circle[0] = centerX + xPomeraj;
    circle[1] = 0.0f;
    circle[2] = centerZ + zPomeraj;

    for (int i = 0; i <= CRES; i++) {
        float angle = (PI / 180) * (i * 360 / CRES);
        circle[3 + 3 * i] = centerX + xPomeraj + r * cos(angle); // Xi pomeren za xPomeraj
        circle[3 + 3 * i + 1] = 0.0f;
        circle[3 + 3 * i + 2] = centerZ + zPomeraj + r * sin(angle); // Zi pomeren za zPomeraj
    }
}

void setXYCircle(float  circle[96], float r, float xPomeraj, float yPomeraj)
{
    float centerX = 0.0;
    float centerY = 0.0;
    float centerZ = 0.0;

    circle[0] = centerX + xPomeraj;
    circle[1] = centerY + yPomeraj;
    circle[2] = 0.0f;


    for (int i = CRES; i >= 0; i--) {
        float angle = (PI / 180) * (i * 360 / CRES);
        circle[3 + 3 * i] = centerX + xPomeraj + r * cos(angle); // Xi pomeren za xPomeraj
        circle[3 + 3 * i + 1] = centerY + yPomeraj + r * sin(angle); // Yi pomeren za yPomeraj
        circle[3 + 3 * i + 2] = 0.0f;
    }
}

unsigned int compileShader(GLenum type, const char* source)
{
    string content = "";
    ifstream file(source);
    stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        cout << "Uspesno procitan fajl sa putanje \"" << source << "\"!" << endl;
    }
    else {
        ss << "";
        cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << endl;
    }
    string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{

    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);


    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        cout << "Objedinjeni sejder ima gresku! Greska: \n";
        cout << infoLog << endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        cout << "Textura nije ucitana! Putanja texture: " << filePath << endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
ModelData loadModel(const char* filePath) {
    ModelData modelData;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cerr << "Error loading model: " << importer.GetErrorString() << endl;
        return modelData;
    }

    processNode(scene->mRootNode, scene, modelData);

    return modelData;
}
void processMesh(aiMesh* mesh, const aiScene* scene, ModelData& modelData) {
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        vec3 vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        modelData.vertices.push_back(vertex);

        if (mesh->HasTextureCoords(0)) {
            vec2 texCoord;
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            modelData.textureCoords.push_back(texCoord);
        }

        if (mesh->HasNormals()) {
            vec3 normal;
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
            modelData.normals.push_back(normal);
        }
        else {
            modelData.normals.push_back(vec3(0.0f, 0.0f, 0.0f));
        }
    }
}
void processNode(aiNode* node, const aiScene* scene, ModelData& modelData) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, modelData);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, modelData);
    }
}