// Autor: Lara Petkoviæ RA 185/2020
// Opis: Protiv-vazdušna odbrana Novog Sada 

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30
#define DRONES_LEFT 7
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

// Za koptere
#include <ctime>
#include <random>
#include <chrono>


unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
void setCircle(float  circle[96], float r, float xPomeraj, float zPomeraj);
static unsigned loadImageToTexture(const char* filePath);
void moveDrone(GLFWwindow* window, float& droneX, float& droneY, float droneSpeed, unsigned int wWidth, unsigned int wHeight);
void generateHelicopterPositions(int number);
void moveHelicoptersTowardsCityCenter(float cityCenterX, float cityCenterY, float speed);
bool checkCollision(float object1X, float object1Y, float object1Radius, float object2X, float object2Y, float object2Radius);
bool isDronOutsideScreen(float droneX, float droneY);

struct Location {
    float x;
    float y;
};

float droneX = 0.0f;
float droneY = -0.45f;
float droneSpeed = 0.0002f;
bool isSpacePressed = false;
bool wasSpacePressed = false;
bool coptersOnScreen = true;
int numberOfCollied = 0;
bool isMapHidden = false;
Location helicopterPositions[5];
auto startTime = std::chrono::high_resolution_clock::now();


int main(void)
{
    if (!glfwInit())
    {
        std::cout << "Greska pri ucitavanju GLFW biblioteke!\n";
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
        std::cout << "Greska pri formiranju prozora!\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK)
    {
        std::cout << "Greska pri ucitavanju GLEW biblioteke!\n";
        return 3;
    }

    unsigned int unifiedShader = createShader("texture.vert", "texture.frag");
    unsigned int baseShader = createShader("base.vert", "base.frag");
    unsigned int dronShader = createShader("dron.vert", "dron.frag");
    int colorLoc = glGetUniformLocation(unifiedShader, "color");

    float vertices[] = {
    -1.0,0.0f, -1.0,  0.0, 0.0,
     1.0,0.0f, -1.0,  1.0, 0.0,
    -1.0,0.0f,  1.0,  0.0, 1.0,

     1.0,0.0f, -1.0,  1.0, 0.0,
     1.0,0.0f,  1.0,  1.0, 1.0
    };

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
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc = glGetUniformLocation(unifiedShader, "uTex");
    glUniform1i(uTexLoc, 0);

    // Opis baze -----------------------------------------------------------------------
    float baseCircle[CRES * 3 + 6];
    setCircle(baseCircle, 0.07, 0.0, -0.45);

    // VAO i VBO baze
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(baseCircle), baseCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    // Opis centra Novog Sada ----------------------------------------------------------
    float cityCenterCircle[CRES * 3 + 6];
    setCircle(cityCenterCircle, 0.017, 0.42, 0.08);

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
    setCircle(blueCircle, 0.03, 0.0, 0.0);

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
    setCircle(LEDBackgroundCircle, 0.045, -0.85, 0.85);

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
    setCircle(LEDCircle, 0.02, -0.85, 0.85);

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

        setCircle(dronLeftCircle, 0.02, 0.7 + 0.04 * i, -0.8);

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

    glm::mat4 model = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    model[0] *= -1;
    unsigned int modelLocTex = glGetUniformLocation(unifiedShader, "uM");
    unsigned int modelLocDron = glGetUniformLocation(dronShader, "uM");
    unsigned int modelLocBase = glGetUniformLocation(baseShader, "uM");


    glm::mat4 view1; //Matrica pogleda (kamere)
    view1 = glm::lookAt(glm::vec3(0.0f, 1.0f, -1.2f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    unsigned int viewLocTex = glGetUniformLocation(unifiedShader, "uV");
    unsigned int viewLocDron = glGetUniformLocation(dronShader, "uV");
    unsigned int viewLocBase = glGetUniformLocation(baseShader, "uV");


    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    unsigned int projectionLocTex = glGetUniformLocation(unifiedShader, "uP");
    unsigned int projectionLocDron = glGetUniformLocation(dronShader, "uP");
    unsigned int projectionLocBase = glGetUniformLocation(baseShader, "uP");
    
    bool wasXpressed = false;

    while (!glfwWindowShouldClose(window))
    {
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

        if ((glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && !wasXpressed && dronesLeft > 0) || isDronOutsideScreen(droneX, droneY)) {
            wasXpressed = true;
            if (!isSpacePressed) {
                wasSpacePressed = !wasSpacePressed;
            }
            isSpacePressed = true;
            droneX = 0.0f;
            droneY = -0.45f;
            dronesLeft--;
        }
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE) {
            wasXpressed = false;
        }


        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(unifiedShader);
        glUniformMatrix4fv(modelLocTex, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
        glUniformMatrix4fv(viewLocTex, 1, GL_FALSE, glm::value_ptr(view1));
        glUniformMatrix4fv(projectionLocTex, 1, GL_FALSE, glm::value_ptr(projectionP));
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


        // Crtanje preostalih dronova
        for (int i = 0; i < dronesLeft; ++i) {
            glBindVertexArray(VAOdronLeft[i]);
            colorLoc = glGetUniformLocation(baseShader, "color");
            glUniform3f(colorLoc, 0.0, 1.0, 0.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(dronLeftCircle) / (3 * sizeof(float)));
        }

        // Renderovanje pozadine LED sijalice
        glUseProgram(baseShader);
        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
        glUniformMatrix4fv(viewLocBase, 1, GL_FALSE, glm::value_ptr(view1));
        glUniformMatrix4fv(projectionLocBase, 1, GL_FALSE, glm::value_ptr(projectionP));
        glBindVertexArray(VAOLEDBackground);
        colorLoc = glGetUniformLocation(baseShader, "color");
        glUniform3f(colorLoc, 0.3, 0.2, 0.2);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(LEDBackgroundCircle) / (3 * sizeof(float)));

        // Renderovanje LED sijalice -> upaljena ako postoji letelica u vazduhu
        glBindVertexArray(VAOLED);
        colorLoc = glGetUniformLocation(baseShader, "color");
        if (coptersOnScreen) {
            glUniform3f(colorLoc, 1.0, 0.0, 0.0); // Boja LED sijalice kada ima helikoptera
        }
        else {
            glUniform3f(colorLoc, 0.0, 1.0, 0.0); // Crna boja LED sijalice kada nema helikoptera
        }
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(LEDCircle) / (3 * sizeof(float)));


        // Renderovanje centra Novog Sada
        //glUseProgram(baseShader);
        glBindVertexArray(VAO[2]);
        glUniform3f(colorLoc, 0.0, 0.0, 0.0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(cityCenterCircle) / (3 * sizeof(float)));


        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!isSpacePressed) {
                wasSpacePressed = !wasSpacePressed;
            }
            isSpacePressed = true;
            droneX = 0.0f;
            droneY = -0.45f;
        }
        else {
            isSpacePressed = false;
        }

        if (wasSpacePressed && dronesLeft > 0)
        {
            moveDrone(window, droneX, droneY, droneSpeed, wWidth, wHeight);
            glUseProgram(dronShader);
            glUniformMatrix4fv(modelLocDron, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
            glUniformMatrix4fv(viewLocDron, 1, GL_FALSE, glm::value_ptr(view1));
            glUniformMatrix4fv(projectionLocDron, 1, GL_FALSE, glm::value_ptr(projectionP));
            glBindVertexArray(VAOBlue);
            GLint translationLoc = glGetUniformLocation(dronShader, "uTranslation");
            glUniform2f(translationLoc, droneX, droneY);
            colorLoc = glGetUniformLocation(dronShader, "color");
            glUniform3f(colorLoc, 0.0, 0.0, 1.0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(blueCircle) / (3 * sizeof(float)));
        }

        // Proteklo vreme od pocetka programa
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

        for (int i = 0; i < 5; i++) {
            // Izraèunamo vektor od helikoptera do centra
            float dirX = 0.42 - helicopterPositions[i].x;
            float dirY = 0.08 - helicopterPositions[i].y;

            // Izraèunamo razdaljinu od koptera do centra - Pitagora
            float distance = sqrt(dirX * dirX + dirY * dirY);

            // Normalizujemo vektor
            dirX /= distance;
            dirY /= distance;

            // Prilagodimo brzinu pulsiranja na osnovu udaljenosti od centra - osnovna brzina je 5.0f
            float pulseSpeed = 5.0f + 10.0f * (1.0f - distance);

            // Izraèunamo faktor pulsiranja na osnovu vremena i udaljenosti
            float pulseFactor = 0.5f + 0.5f * sin(elapsedTime * pulseSpeed);
            float redIntensity = 1.0;
            float greenIntensity = 1.0 - pulseFactor;
            float blueIntensity = 1.0 - pulseFactor;

            glUseProgram(dronShader);
            glUniformMatrix4fv(modelLocDron, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
            glUniformMatrix4fv(viewLocDron, 1, GL_FALSE, glm::value_ptr(view1));
            glUniformMatrix4fv(projectionLocDron, 1, GL_FALSE, glm::value_ptr(projectionP));
            glBindVertexArray(VAOBlue);
            GLint translationLoc = glGetUniformLocation(dronShader, "uTranslation");
            glUniform2f(translationLoc, helicopterPositions[i].x, helicopterPositions[i].y);
            colorLoc = glGetUniformLocation(dronShader, "color");
            glUniform3f(colorLoc, redIntensity, greenIntensity, blueIntensity);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(blueCircle) / (3 * sizeof(float)));

            if (checkCollision(droneX, droneY, 0.03, helicopterPositions[i].x, helicopterPositions[i].y, 0.03)) {
                droneX = 0.0f; // Resetovanje pozicije drona
                droneY = -0.45f;
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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &mapTexture);
    glDeleteBuffers(3, VBO);
    glDeleteVertexArrays(3, VAO);
    glDeleteBuffers(1, &VBOBlue);
    glDeleteVertexArrays(1, &VAOBlue);
    glDeleteBuffers(1, &VBOLED);
    glDeleteVertexArrays(1, &VAOLED);
    glDeleteBuffers(1, &VBOLEDBackground);
    glDeleteVertexArrays(1, &VAOLEDBackground);
    glDeleteProgram(unifiedShader);
    glDeleteProgram(baseShader);

    for (int i = 0; i < DRONES_LEFT; i++) {
        glDeleteVertexArrays(1, &VAOdronLeft[i]);
        glDeleteBuffers(1, &VBOdronLeft[i]);
    }

    glfwTerminate();
    return 0;
}

bool checkCollision(float object1X, float object1Y, float object1Radius, float object2X, float object2Y, float object2Radius) {
    float distance = std::sqrt(std::pow(object2X - object1X, 2) + std::pow(object2Y - object1Y, 2));
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
        if (strana == 0) {
            helicopterPositions[i].x = 1;
            std::string randomFloat = "0.";
            randomFloat.append(std::to_string(rand() % 10));
            randomFloat.append(std::to_string(rand() % 10));
            helicopterPositions[i].y = std::stof(randomFloat);
        }
        else if (strana == 1) {
            std::string randomFloat = "0.";
            randomFloat.append(std::to_string(rand() % 10));
            randomFloat.append(std::to_string(rand() % 10));
            helicopterPositions[i].x = std::stof(randomFloat);
            helicopterPositions[i].y = 1;
        }
        else if (strana == 2) {
            helicopterPositions[i].x = -1;
            std::string randomFloat = "0.";
            randomFloat.append(std::to_string(rand() % 10));
            randomFloat.append(std::to_string(rand() % 10));
            helicopterPositions[i].y = std::stof(randomFloat);
        }
        else {
            std::string randomFloat = "0.";
            randomFloat.append(std::to_string(rand() % 10));
            randomFloat.append(std::to_string(rand() % 10));
            helicopterPositions[i].x = std::stof(randomFloat);
            helicopterPositions[i].y = -1;
        }
    }
}

bool isDronOutsideScreen(float droneX, float droneY)
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

void setCircle(float  circle[96], float r, float xPomeraj, float zPomeraj)
{
    float centerX = 0.0;
    float centerY = 0.0;
    float centerZ = 0.0;

    circle[0] = centerX + xPomeraj;
    circle[1] = 0.0f;
    circle[2] = centerZ + zPomeraj;

    for (int i = 0; i <= CRES; i++) {
        circle[3 + 3 * i] = centerX + xPomeraj + r * cos((3.141592 / 180) * (i * 360 / CRES)); // Xi pomeren za xPomeraj
        circle[3 + 3 * i + 1] = 0.0f;
        circle[3 + 3 * i + 2] = centerY + zPomeraj + r * sin((3.141592 / 180) * (i * 360 / CRES)); // Zi pomeren za yPomeraj
    }
    //Crtali smo od "nultog" ugla ka jednom pravcu, sto nam ostavlja prazno mesto od poslednjeg temena kruznice do prvog,
    //pa da bi ga zatvorili, koristili smo <= umesto <, sto nam dodaje tjeme (cos(0), sin(0))
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspesno procitan fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
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
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
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
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}