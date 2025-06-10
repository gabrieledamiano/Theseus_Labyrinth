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
#include "sword.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace irrklang;



struct GameContext {
    Sword* sword;
    //Minotaur* minotaur;
    Camera* camera;
    // In futuro potrai aggiungere altri puntatori qui
};

//--- Variabili Globali ---
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundSource* mainTheme = SoundEngine->addSoundSourceFromFile("resources/main.mp3");
ISoundSource* attackSound = SoundEngine->addSoundSourceFromFile("resources/sword_swing.mp3");

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

bool isGameActive = false;

const int MAP_SIZE_ROWS = 40;
const int MAP_SIZE_COLS = 20;
const int MAZE_WIDTH = MAP_SIZE_COLS;
const int MAZE_HEIGHT = MAP_SIZE_ROWS;
const float CELL_SIZE = 1.5f;
const float WALL_HEIGHT = 3.0f;
const float CAMERA_HEIGHT = 1.5f;
const float CAMERA_COLLISION_RADIUS = 0.2f;

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
int itemsFound = 0;

unsigned int VBO_cube_lit, VAO_walls, VAO_floor, VAO_ceiling;
unsigned int VAO_lamp, VAO_window, VBO_window, menuVAO, menuVBO;
unsigned int textureWall, textureFloor, textureCeiling, textureWindow, menuTexture;

const int NR_SPOT_LIGHTS = 12;
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

// Dichiarazioni funzioni
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void loadMazeFromMap();
bool checkCollision(glm::vec3 checkPos);
void setupMazeGeometryVAOs();
void setupWindowVAO();
void setupMenuVAO();
unsigned int loadtexture(const std::string& path, bool clampToEdge = false);

inline unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Labirinto del Minotauro", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    initRenderText(SCR_WIDTH, SCR_HEIGHT);
    stbi_set_flip_vertically_on_load(false);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader modelShader("model.vs", "model.fs");
    Shader mazeShader("spot_light.vs", "spot_light.fs");
    Shader lampShader("lamp.vs", "lamp.fs");
    Shader menuShader("menu.vs", "menu.fs");

    Sword sword("resources/sword/sword.obj");

    loadMazeFromMap();
    camera.Position = glm::vec3(1 * CELL_SIZE + CELL_SIZE / 2.0f, CAMERA_HEIGHT, 1 * CELL_SIZE + CELL_SIZE / 2.0f);

    float lightHeight = WALL_HEIGHT - 0.1f;
    spotLightPositions[0] = glm::vec3(1.5f * CELL_SIZE, lightHeight, 1.5f * CELL_SIZE);
    if (NR_SPOT_LIGHTS > 1) spotLightPositions[1] = glm::vec3(8.73f, lightHeight, 5.30f);
    if (NR_SPOT_LIGHTS > 2) spotLightPositions[2] = glm::vec3(10.16f, lightHeight, 14.29f);
    if (NR_SPOT_LIGHTS > 3) spotLightPositions[3] = glm::vec3(8.04f, lightHeight, 23.15f);
    if (NR_SPOT_LIGHTS > 4) spotLightPositions[4] = glm::vec3(24.24f, lightHeight, 2.99f);
    if (NR_SPOT_LIGHTS > 5) spotLightPositions[5] = glm::vec3(14.11f, lightHeight, 14.81f);
    if (NR_SPOT_LIGHTS > 6) spotLightPositions[6] = glm::vec3(23.33f, lightHeight, 14.56f);
    if (NR_SPOT_LIGHTS > 7) spotLightPositions[7] = glm::vec3(26.92f, lightHeight, 29.50f);
    if (NR_SPOT_LIGHTS > 8) spotLightPositions[8] = glm::vec3(24.00f, lightHeight, 2.00f);
    if (NR_SPOT_LIGHTS > 9) spotLightPositions[9] = glm::vec3(1.77f, lightHeight, 19.19f);
    if (NR_SPOT_LIGHTS > 10) spotLightPositions[10] = glm::vec3(10.15f, lightHeight, 14.72f);
    if (NR_SPOT_LIGHTS > 11) spotLightPositions[11] = glm::vec3(7.92f, lightHeight, 3.38f);

    textureWall = loadtexture("resources/textures/wall_diffuse.jpg", false);
    textureFloor = loadtexture("resources/textures/floor_diffuse.jpg", false);
    textureCeiling = loadtexture("resources/textures/ceiling.jpg", false);
    menuTexture = loadtexture("resources/textures/menu.jpg", false);

    setupMenuVAO();
    setupMazeGeometryVAOs();

    menuShader.use();
    menuShader.setInt("menuTexture", 0);
    mazeShader.use();
    mazeShader.setInt("material.diffuse", 0);
    mazeShader.setFloat("material.shininess", mazeShininess);

    if (mainTheme) {
        SoundEngine->play2D(mainTheme, true);
    }

    glfwSetWindowUserPointer(window, &sword);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (isGameActive)
        {
            sword.Update(deltaTime);

            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            mazeShader.use();
            mazeShader.setMat4("projection", projection);
            mazeShader.setMat4("view", view);
            mazeShader.setVec3("viewPos", camera.Position);
            mazeShader.setInt("activeSpotLights", NR_SPOT_LIGHTS);
            for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
                string lightUni = "spotLights[" + std::to_string(i) + "]";
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

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureFloor);
            glBindVertexArray(VAO_floor);
            mazeShader.setMat4("model", glm::mat4(1.0f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureCeiling);
            glBindVertexArray(VAO_ceiling);
            mazeShader.setMat4("model", glm::mat4(1.0f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureWall);
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

            sword.Draw(modelShader, camera);

            glDisable(GL_DEPTH_TEST);
            std::string itemsInfo = "Oggetti Raccolti: " + std::to_string(itemsFound) + "/3";
            RenderText(itemsInfo.c_str(), 10.0f, SCR_HEIGHT - 40.0f, 0.7f, glm::vec3(1.0, 1.0, 0.0));
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
            menuShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, menuTexture);
            glBindVertexArray(menuVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
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
    glDeleteBuffers(1, &VBO_cube_lit);
    glDeleteTextures(1, &textureWall);
    glDeleteTextures(1, &textureFloor);
    glDeleteTextures(1, &textureCeiling);
    glDeleteVertexArrays(1, &menuVAO);
    glDeleteBuffers(1, &menuVBO);
    glDeleteTextures(1, &menuTexture);

    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (!isGameActive) return;

    Sword* sword = static_cast<Sword*>(glfwGetWindowUserPointer(window));

    if (sword && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        sword->Attack();
        if (attackSound)
            SoundEngine->play2D(attackSound);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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
    if (isGameActive)
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (!isGameActive) {
        static bool enterPressedLastFrame = false;
        bool enterIsPressed = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

        if (enterIsPressed && !enterPressedLastFrame) {
            isGameActive = true;
            SoundEngine->stopAllSounds();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        enterPressedLastFrame = enterIsPressed;
    }
    else {
        float desiredHeight = CAMERA_HEIGHT;
        glm::vec3 originalPosition = camera.Position;

        float moveSpeed = 3.5f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime * moveSpeed);

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
    if (checkPos.y < CAMERA_HEIGHT - 0.1f || checkPos.y > CAMERA_HEIGHT + 0.1f) return true;

    float radius = CAMERA_COLLISION_RADIUS;
    int gridX = static_cast<int>(floor(checkPos.x / CELL_SIZE));
    int gridZ = static_cast<int>(floor(checkPos.z / CELL_SIZE));

    for (int z = gridZ - 1; z <= gridZ + 1; ++z) {
        for (int x = gridX - 1; x <= gridX + 1; ++x) {
            if (x >= 0 && x < MAZE_WIDTH && z >= 0 && z < MAZE_HEIGHT && maze[z][x].wall) {
                float wallX = (x * CELL_SIZE) + (CELL_SIZE / 2.0f);
                float wallZ = (z * CELL_SIZE) + (CELL_SIZE / 2.0f);

                float closestX = max(wallX - CELL_SIZE / 2.0f, min(checkPos.x, wallX + CELL_SIZE / 2.0f));
                float closestZ = max(wallZ - CELL_SIZE / 2.0f, min(checkPos.z, wallZ + CELL_SIZE / 2.0f));

                float distance = sqrt(pow(closestX - checkPos.x, 2) + pow(closestZ - checkPos.z, 2));

                if (distance < radius) {
                    return true;
                }
            }
        }
    }
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

    float ceilingVertices[] = {
        mazeW, WALL_HEIGHT, mazeD, 0.0f, -1.0f, 0.0f, textureRepeatX, textureRepeatZ,
        0.0f,  WALL_HEIGHT, mazeD, 0.0f, -1.0f, 0.0f, 0.0f, textureRepeatZ,
        0.0f,  WALL_HEIGHT, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        mazeW, WALL_HEIGHT, 0.0f,  0.0f, -1.0f, 0.0f, textureRepeatX, 0.0f
    };
    unsigned int VBO_ceiling, EBO_ceiling;
    glGenVertexArrays(1, &VAO_ceiling);
    glGenBuffers(1, &VBO_ceiling);
    glGenBuffers(1, &EBO_ceiling);
    glBindVertexArray(VAO_ceiling);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ceiling);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ceilingVertices), ceilingVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ceiling);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW); // Re-using planeIndices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &VAO_lamp);
    glBindVertexArray(VAO_lamp);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void setupMenuVAO() {
    float menuVertices[] = { -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
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
        else { stbi_image_free(data); return 0; }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLint wrapMode = clampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}