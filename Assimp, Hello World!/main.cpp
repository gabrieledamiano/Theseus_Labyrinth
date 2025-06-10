#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include <map>
#include <irrklang/irrKlang.h>
#include "render_text.h"

#pragma comment(lib, "irrKlang.lib")

#include "shader_m.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace irrklang;

// --- Aggiunto da MAIN 1 ---
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundSource* mainTheme = SoundEngine->addSoundSourceFromFile("resources/main.mp3");
//ISoundSource* attackSound = SoundEngine->addSoundSourceFromFile("resources/sword_swing.mp3");
//ISoundSource* therdSound = SoundEngine->addSoundSourceFromFile("resources/chiave.mp3");
//ISound* ambientSound;
// --- Fine aggiunta ---

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// --- MODIFICA: Variabile di stato del gioco ---
bool isGameActive = false;

const int MAP_SIZE_ROWS = 40;
const int MAP_SIZE_COLS = 20;

const int MAZE_WIDTH = MAP_SIZE_COLS;
const int MAZE_HEIGHT = MAP_SIZE_ROWS;

const float CELL_SIZE = 1.5f;
const float WALL_HEIGHT = 3.0f;

const float CAMERA_HEIGHT = 1.5f;
const float CAMERA_COLLISION_RADIUS = 0.2f;
const float MIN_CAMERA_Y = 0.1f;
const float MAX_CAMERA_Y = WALL_HEIGHT - 0.1f;

const int initial_maze_map[MAP_SIZE_ROWS][MAP_SIZE_COLS] = {
{ 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1 },
{ 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
{ 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
{ 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 },
{ 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
{ 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
{ 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
{ 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
{ 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
{ 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1 },
{ 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
{ 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },
{ 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

struct Cell { bool visited = false; bool wall = true; };
std::vector<std::vector<Cell>> maze(MAZE_HEIGHT, std::vector<Cell>(MAZE_WIDTH));

Camera camera(glm::vec3(0.0f, CAMERA_HEIGHT, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- Aggiunto per il testo ---
int itemsFound = 0;
// --- Fine aggiunta ---


unsigned int VBO_cube_lit, VAO_walls, VAO_floor, VAO_ceiling;
unsigned int VAO_lamp;
unsigned int VAO_window, VBO_window;
unsigned int textureWall, textureFloor, textureCeiling, textureSpecularMaze;
unsigned int textureWindow;

// --- MODIFICA: Aggiunti VAO e texture per il menu ---
unsigned int menuVAO, menuVBO;
unsigned int menuTexture;

const int NR_SPOT_LIGHTS = 20;
glm::vec3 spotLightPositions[NR_SPOT_LIGHTS];
glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 spotLightAmbient = glm::vec3(0.4f, 0.2f, 0.1f);
glm::vec3 spotLightSpecular = glm::vec3(0.4f, 0.0f, 0.0f);
glm::vec3 spotLightDiffuse = glm::vec3(0.6f, 0.0f, 0.0f);
float spotLightConstant = 1.0f;
float spotLightLinear = 0.07f;
float spotLightQuadratic = 0.017f;
float spotLightCutOff = glm::cos(glm::radians(54.0f));
float spotLightOuterCutOff = glm::cos(glm::radians(88.0f));
float mazeShininess = 32.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void loadMazeFromMap();
bool checkCollision(glm::vec3 checkPos);
void setupMazeGeometryVAOs();
void setupWindowVAO();
void setupMenuVAO();
unsigned int loadtexture(const std::string& path, bool clampToEdge = false);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Labirinto", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // --- Aggiunto da MAIN 1 ---
    initRenderText(SCR_WIDTH, SCR_HEIGHT);
    // --- Fine aggiunta ---

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    loadMazeFromMap();

    camera.Position = glm::vec3(1 * CELL_SIZE + CELL_SIZE / 2.0f, CAMERA_HEIGHT, 1 * CELL_SIZE + CELL_SIZE / 2.0f);

    float lightHeight = WALL_HEIGHT - 0.1f;
    spotLightPositions[0] = glm::vec3(1.5f * CELL_SIZE, lightHeight, 1.5f * CELL_SIZE);
    /* if (NR_SPOT_LIGHTS > 1) spotLightPositions[1] = glm::vec3(8.73f, lightHeight, 5.30f);
     if (NR_SPOT_LIGHTS > 2) spotLightPositions[2] = glm::vec3(10.16f, lightHeight, 14.29f);
     if (NR_SPOT_LIGHTS > 3) spotLightPositions[3] = glm::vec3(8.04f, lightHeight, 23.15f);
     if (NR_SPOT_LIGHTS > 4) spotLightPositions[4] = glm::vec3(24.24f, lightHeight, 2.99f);
     if (NR_SPOT_LIGHTS > 5) spotLightPositions[5] = glm::vec3(14.11f, lightHeight, 14.81f);
     if (NR_SPOT_LIGHTS > 6) spotLightPositions[6] = glm::vec3(23.33f, lightHeight, 14.56f);
     if (NR_SPOT_LIGHTS > 7) spotLightPositions[7] = glm::vec3(26.92f, lightHeight, 29.50f);*/
    if (NR_SPOT_LIGHTS > 8) spotLightPositions[8] = glm::vec3(24.00f, lightHeight, 2.00f);
    if (NR_SPOT_LIGHTS > 9) spotLightPositions[9] = glm::vec3(1.77f, lightHeight, 19.19f);
    if (NR_SPOT_LIGHTS > 10) spotLightPositions[10] = glm::vec3(10.15f, lightHeight, 14.72f);
    if (NR_SPOT_LIGHTS > 11) spotLightPositions[11] = glm::vec3(7.92f, lightHeight, 3.38f);

    std::vector<glm::vec3> windowPositions;
    float windowY = WALL_HEIGHT / 2.0f;
    windowPositions.push_back(glm::vec3(2.25f, windowY, 0.20f));
    windowPositions.push_back(glm::vec3(29.25f, windowY, 12.72f));
    windowPositions.push_back(glm::vec3(29.25f, windowY, 31.29f));

    textureWall = loadtexture("resources/textures/wall_diffuse.jpg");
    textureFloor = loadtexture("resources/textures/floor_diffuse.jpg");
    textureCeiling = loadtexture("resources/textures/ceiling.jpg");
    //textureSpecularMaze = loadtexture("resources/textures/normal2.jpg");
    textureWindow = loadtexture("resources/textures/window.png", true);

    // --- MODIFICA: Caricamento texture e setup VAO del menu ---
    menuTexture = loadtexture("resources/textures/menu.jpg", false);
    setupMenuVAO();

    if (textureWall == 0 || textureFloor == 0 || textureCeiling == 0 /*|| textureSpecularMaze == 0*/ || textureWindow == 0 || menuTexture == 0) {
        std::cerr << "Errore caricamento texture." << std::endl;
        glfwTerminate(); return -1;
    }

    setupMazeGeometryVAOs();
    setupWindowVAO();

    Shader mazeShader("spot_light.vs", "spot_light.fs");
    Shader lampShader("lamp.vs", "lamp.fs");
    Shader windowShader("blending.vs", "blending.fs");

    // --- MODIFICA: Creazione shader per il menu ---
    Shader menuShader("menu.vs", "menu.fs");
    menuShader.use();
    menuShader.setInt("menuTexture", 0);

    mazeShader.use();
    mazeShader.setInt("material.diffuse", 0);
    mazeShader.setInt("material.specular", 1);
    mazeShader.setFloat("material.shininess", mazeShininess);
    windowShader.use();
    windowShader.setInt("texture1", 0);

    // Musica menu
    if (mainTheme) {
        SoundEngine->play2D(mainTheme, true);
    }

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- MODIFICA: Logica di rendering condizionale ---
        if (isGameActive)
        {
            // --- INIZIO CODICE DI GIOCO ---
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            mazeShader.use();
            mazeShader.setMat4("projection", projection);
            mazeShader.setMat4("view", view);
            mazeShader.setVec3("viewPos", camera.Position);
            mazeShader.setInt("activeSpotLights", NR_SPOT_LIGHTS);
            for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
                std::string lightUni = "spotLights[" + std::to_string(i) + "]";
                mazeShader.setVec3(lightUni + ".position", spotLightPositions[i]);
                mazeShader.setVec3(lightUni + ".direction", spotLightDirection);
                mazeShader.setFloat(lightUni + ".cutOff", spotLightCutOff);
                mazeShader.setFloat(lightUni + ".outerCutOff", spotLightOuterCutOff);
                mazeShader.setVec3(lightUni + ".ambient", spotLightAmbient);
                mazeShader.setVec3(lightUni + ".diffuse", spotLightDiffuse);
                mazeShader.setVec3(lightUni + ".specular", spotLightSpecular);
                mazeShader.setFloat(lightUni + ".constant", spotLightConstant);
                mazeShader.setFloat(lightUni + ".linear", spotLightLinear);
                mazeShader.setFloat(lightUni + ".quadratic", spotLightQuadratic);
            }

            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureFloor);
            glBindVertexArray(VAO_floor);
            mazeShader.setMat4("model", glm::mat4(1.0f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureCeiling);
            glBindVertexArray(VAO_ceiling);
            mazeShader.setMat4("model", glm::mat4(1.0f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, textureWall);
            glBindVertexArray(VAO_walls);
            for (int y = 0; y < MAZE_HEIGHT; ++y) {
                for (int x = 0; x < MAZE_WIDTH; ++x) {
                    if (maze[y][x].wall) {
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3(x * CELL_SIZE + CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, y * CELL_SIZE + CELL_SIZE / 2.0f));
                        model = glm::scale(model, glm::vec3(CELL_SIZE, WALL_HEIGHT, CELL_SIZE));
                        mazeShader.setMat4("model", model);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
            glBindVertexArray(0);

            lampShader.use();
            lampShader.setMat4("projection", projection);
            lampShader.setMat4("view", view);
            glBindVertexArray(VAO_lamp);
            for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, spotLightPositions[i]);
                model = glm::scale(model, glm::vec3(0.15f));
                lampShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindVertexArray(0);

            std::map<float, glm::vec3> sortedWindows;
            for (const auto& pos : windowPositions) {
                float distance = glm::length(camera.Position - pos);
                sortedWindows[distance] = pos;
            }

            windowShader.use();
            windowShader.setMat4("projection", projection);
            windowShader.setMat4("view", view);
            glBindVertexArray(VAO_window);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureWindow);

            for (auto it = sortedWindows.rbegin(); it != sortedWindows.rend(); ++it) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, it->second);
                model = glm::scale(model, glm::vec3(CELL_SIZE * 0.8f, WALL_HEIGHT * 0.8f, 1.0f));
                windowShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            glBindVertexArray(0);

            glDisable(GL_DEPTH_TEST);
            std::string itemsInfo = "Oggetti Raccolti: " + std::to_string(itemsFound) + "/3";
            RenderText(itemsInfo.c_str(), 10.0f, SCR_HEIGHT - 40.0f, 0.7f, glm::vec3(1.0, 1.0, 0.0));
            glEnable(GL_DEPTH_TEST);
            // --- FINE CODICE DI GIOCO ---
        }
        else
        {
            // --- CODICE DEL MENU ---
            glDisable(GL_DEPTH_TEST); // Il menu non ha bisogno del depth test
            menuShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(menuVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    SoundEngine->drop();

    glDeleteVertexArrays(1, &VAO_walls);
    glDeleteVertexArrays(1, &VAO_floor);
    glDeleteVertexArrays(1, &VAO_ceiling);
    glDeleteVertexArrays(1, &VAO_lamp);
    glDeleteVertexArrays(1, &VAO_window);
    glDeleteBuffers(1, &VBO_cube_lit);
    glDeleteBuffers(1, &VBO_window);
    glDeleteTextures(1, &textureWall);
    glDeleteTextures(1, &textureFloor);
    glDeleteTextures(1, &textureCeiling);
   // glDeleteTextures(1, &textureSpecularMaze);
    glDeleteTextures(1, &textureWindow);
    // --- MODIFICA: Pulizia risorse menu ---
    glDeleteVertexArrays(1, &menuVAO);
    glDeleteBuffers(1, &menuVBO);
    glDeleteTextures(1, &menuTexture);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    // --- MODIFICA: Il mouse muove la camera solo se il gioco è attivo ---
    if (!isGameActive) {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // --- MODIFICA: Lo scroll funziona solo se il gioco è attivo ---
    if (isGameActive)
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // --- MODIFICA: Logica di input separata per menu e gioco ---
    if (!isGameActive) {
        // Siamo nel menu, aspettiamo solo INVIO
        static bool enterPressedLastFrame = false;
        bool enterIsPressed = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

        if (enterIsPressed && !enterPressedLastFrame) {
            isGameActive = true;
            // Nascondi il cursore e catturalo per il gioco
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // Resetta il firstMouse per evitare scatti della camera
        }
        enterPressedLastFrame = enterIsPressed;
    }
    else {
        // Il gioco è attivo, gestisci i normali controlli
        float desiredHeight = CAMERA_HEIGHT;
        glm::vec3 originalPosition = camera.Position;

        float moveSpeed = 3.5f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime * moveSpeed);

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            std::cout << "Pos Camera: X=" << camera.Position.x << " Z=" << camera.Position.z << std::endl;
            int camGridX = static_cast<int>(floor(camera.Position.x / CELL_SIZE));
            int camGridZ = static_cast<int>(floor(camera.Position.z / CELL_SIZE));
            std::cout << "     -> Cella circa: (" << camGridX << ", " << camGridZ << ")" << std::endl;
        }

        glm::vec3 attemptedPosition = camera.Position;
        attemptedPosition.y = desiredHeight;

        if (checkCollision(attemptedPosition)) {
            glm::vec3 checkPosNoX = glm::vec3(originalPosition.x, desiredHeight, attemptedPosition.z);
            if (!checkCollision(checkPosNoX)) {
                camera.Position.x = originalPosition.x;
            }
            else {
                glm::vec3 checkPosNoZ = glm::vec3(attemptedPosition.x, desiredHeight, originalPosition.z);
                if (!checkCollision(checkPosNoZ)) {
                    camera.Position.z = originalPosition.z;
                }
                else {
                    camera.Position.x = originalPosition.x;
                    camera.Position.z = originalPosition.z;
                }
            }
        }
        camera.Position.y = desiredHeight;
    }
}

void loadMazeFromMap() {
    for (int y = 0; y < MAZE_HEIGHT; ++y) {
        for (int x = 0; x < MAZE_WIDTH; ++x) {
            maze[y][x].visited = false;
            maze[y][x].wall = (initial_maze_map[y][x] == 1);
        }
    }
}

bool checkCollision(glm::vec3 checkPos) {
    if (checkPos.y < MIN_CAMERA_Y) return true;
    if (checkPos.y > MAX_CAMERA_Y) return true;

    float radius = CAMERA_COLLISION_RADIUS;
    int gridX_min = static_cast<int>(floor((checkPos.x - radius) / CELL_SIZE));
    int gridZ_min = static_cast<int>(floor((checkPos.z - radius) / CELL_SIZE));
    int gridX_max = static_cast<int>(floor((checkPos.x + radius) / CELL_SIZE));
    int gridZ_max = static_cast<int>(floor((checkPos.z + radius) / CELL_SIZE));

    auto isCellWall = [&](int x, int z) {
        if (x < 0 || x >= MAZE_WIDTH || z < 0 || z >= MAZE_HEIGHT) return true;
        return maze[z][x].wall;
        };

    if (isCellWall(gridX_min, gridZ_min)) return true;
    if (isCellWall(gridX_max, gridZ_min)) return true;
    if (isCellWall(gridX_min, gridZ_max)) return true;
    if (isCellWall(gridX_max, gridZ_max)) return true;

    return false;
}

void setupMazeGeometryVAOs() {
    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
    };
    glGenBuffers(1, &VBO_cube_lit);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO_walls);
    glBindVertexArray(VAO_walls);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    float mazeW = (float)MAZE_WIDTH * CELL_SIZE;
    float mazeD = (float)MAZE_HEIGHT * CELL_SIZE;
    float textureRepeatX = mazeW / CELL_SIZE;
    float textureRepeatZ = mazeD / CELL_SIZE;

    float floorVertices[] = {
        mazeW, 0.0f, mazeD, 0.0f, 1.0f, 0.0f, textureRepeatX, textureRepeatZ,
        mazeW, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, textureRepeatX, 0.0f,
        0.0f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f,  0.0f, mazeD, 0.0f, 1.0f, 0.0f, 0.0f, textureRepeatZ
    };
    unsigned int planeIndices[] = { 0, 1, 3, 1, 2, 3 };
    unsigned int VBO_floor, EBO_floor;
    glGenVertexArrays(1, &VAO_floor);
    glGenBuffers(1, &VBO_floor);
    glGenBuffers(1, &EBO_floor);
    glBindVertexArray(VAO_floor);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_floor);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    float ceilingVertices[] = {
        mazeW, WALL_HEIGHT, mazeD, 0.0f, -1.0f, 0.0f, textureRepeatX, textureRepeatZ,
        0.0f,  WALL_HEIGHT, mazeD, 0.0f, -1.0f, 0.0f, 0.0f, textureRepeatZ,
        0.0f,  WALL_HEIGHT, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        mazeW, WALL_HEIGHT, 0.0f,  0.0f, -1.0f, 0.0f, textureRepeatX, 0.0f
    };
    unsigned int ceilingIndices[] = { 0, 3, 1, 1, 3, 2 };
    unsigned int VBO_ceiling, EBO_ceiling;
    glGenVertexArrays(1, &VAO_ceiling);
    glGenBuffers(1, &VBO_ceiling);
    glGenBuffers(1, &EBO_ceiling);
    glBindVertexArray(VAO_ceiling);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ceiling);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ceilingVertices), ceilingVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ceiling);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ceilingIndices), ceilingIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    glGenVertexArrays(1, &VAO_lamp);
    glBindVertexArray(VAO_lamp);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void setupWindowVAO() {
    float transparentVertices[] = {
          0.5f,  0.5f,  0.0f,  1.0f, 1.0f,
         -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.0f,  0.0f, 0.0f,
         -0.5f, -0.5f,  0.0f,  0.0f, 0.0f,
          0.5f, -0.5f,  0.0f,  1.0f, 0.0f,
          0.5f,  0.5f,  0.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &VAO_window);
    glGenBuffers(1, &VBO_window);
    glBindVertexArray(VAO_window);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_window);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

// --- MODIFICA: Nuova funzione per creare la geometria del menu ---
void setupMenuVAO() {
    float menuVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &menuVAO);
    glGenBuffers(1, &menuVBO);
    glBindVertexArray(menuVAO);
    glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(menuVertices), &menuVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

unsigned int loadtexture(const std::string& path, bool clampToEdge) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        else {
            std::cerr << "Formato immagine non supportato: " << path << std::endl;
            stbi_image_free(data);
            return 0;
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        GLint wrapMode = (format == GL_RGBA && clampToEdge) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cerr << "Errore caricamento texture: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }
    return textureID;
}