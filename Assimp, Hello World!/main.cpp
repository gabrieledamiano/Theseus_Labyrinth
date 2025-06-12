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
#include "cell.h" 
#include "config.h"

#pragma comment(lib, "irrKlang.lib")

#include "shader_m.h"
#include "camera.h"
#include "sword.h"
#include "animator.h"
#include "minotaur.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace irrklang;

struct GameContext {
    Sword* sword;
    Minotaur* minotaur;
    Camera* camera;
    // In futuro potrai aggiungere altri puntatori qui
};

//--- Variabili Globali ---
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundSource* mainTheme = SoundEngine->addSoundSourceFromFile("resources/main.mp3");
ISoundSource* attackSound = SoundEngine->addSoundSourceFromFile("resources/sword_swing.mp3");
ISoundSource* minotaurHitSound = SoundEngine->addSoundSourceFromFile("resources/hit.mp3");
ISoundSource* minotaurDeathSound = SoundEngine->addSoundSourceFromFile("resources/death.mp3");

//const unsigned int SCR_WIDTH = 1920;
//const unsigned int SCR_HEIGHT = 1080;

bool isGameActive = false;

// --- SALUTE GIOCATORE: Variabili per la salute ---
float playerHealth = 100.0f;
const float PLAYER_MAX_HEALTH = 100.0f;
bool isPlayerDead = false;

//const int MAP_SIZE_ROWS = 40;
//const int MAP_SIZE_COLS = 20;
//const int MAZE_WIDTH = MAP_SIZE_COLS;
//const int MAZE_HEIGHT = MAP_SIZE_ROWS;
//const float CELL_SIZE = 1.5f;
//const float WALL_HEIGHT = 3.0f;
//const float CAMERA_HEIGHT = 1.5f;
//const float CAMERA_COLLISION_RADIUS = 0.2f;

const int initial_maze_map[MAP_SIZE_ROWS][MAP_SIZE_COLS] = {
    { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1 },
    { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
    { 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 },
    { 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { 1, 2, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
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

// New texture variables for normal mapping
unsigned int textureWall, textureFloor, textureCeiling, textureWindow, menuTexture;
unsigned int textureNormalWall, textureNormalFloor, textureNormalCeiling;


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
void loadLevelData(glm::vec3& minotaurSpawnPos);
bool checkCollision(glm::vec3 checkPos, const Minotaur& minotaur);
void setupMazeGeometryVAOs();
void setupWindowVAO();
void setupMenuVAO();
unsigned int loadtexture(const std::string& path, bool clampToEdge = false);
void PlayerTakeDamage(float damage);

// Added inline function for loading textures from file (from second main)
inline unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
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
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Theseus' Labyrinth", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Setting cursor to normal initially for the menu screen
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    initRenderText(SCR_WIDTH, SCR_HEIGHT);
    stbi_set_flip_vertically_on_load(false); // Keep this false as per your standard main

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader modelShader("model.vs", "model.fs");
    // The mazeShader will now use the updated spot_light shaders with normal mapping
    Shader mazeShader("spot_light.vs", "spot_light.fs");
    Shader lampShader("lamp.vs", "lamp.fs");
    Shader menuShader("menu.vs", "menu.fs");
    Shader animModelShader("anim_model.vs", "anim_model.fs"); // Per il minotauro

    Sword sword("resources/sword/sword.obj");
    Model minotaurModel("resources/minotaur/minotaur.glb"); // Assicurati di avere il modello del minotauro
    minotaurModel.LoadAnimation("idle", "resources/minotaur/idle.glb");
    minotaurModel.LoadAnimation("walk", "resources/minotaur/walk.glb");
    minotaurModel.LoadAnimation("get_hit", "resources/minotaur/get_hit.glb");
    minotaurModel.LoadAnimation("death", "resources/minotaur/death.glb");
    minotaurModel.LoadAnimation("attack", "resources/minotaur/attack.glb");

    // 1. Dichiara una variabile per la posizione di partenza
    glm::vec3 minotaurStartPosition;
    // 2. Chiama la nuova funzione per caricare il labirinto E trovare lo spawn
    loadLevelData(minotaurStartPosition);

    camera.Position = glm::vec3(1 * CELL_SIZE + CELL_SIZE / 2.0f, CAMERA_HEIGHT, 1 * CELL_SIZE + CELL_SIZE / 2.0f);
    Minotaur minotaur(minotaurModel, glm::vec3(15.0f * CELL_SIZE, 0.0f, 15.0f * CELL_SIZE), maze); //per il minotauro
    minotaur.position = minotaurStartPosition; // Set minotaur's actual spawn position

    float lightHeight = WALL_HEIGHT - 0.1f;
    spotLightPositions[0] = glm::vec3(1.5f * CELL_SIZE, lightHeight, 1.5f * CELL_SIZE);
    if (NR_SPOT_LIGHTS > 1) spotLightPositions[1] = glm::vec3(24.00f, lightHeight, 2.00f);
    if (NR_SPOT_LIGHTS > 2) spotLightPositions[2] = glm::vec3(1.77f, lightHeight, 19.19f);
    if (NR_SPOT_LIGHTS > 3) spotLightPositions[3] = glm::vec3(10.15f, lightHeight, 14.72f);
    if (NR_SPOT_LIGHTS > 4) spotLightPositions[4] = glm::vec3(7.92f, lightHeight, 3.38f);
    if (NR_SPOT_LIGHTS > 5) spotLightPositions[5] = glm::vec3(9.35f, lightHeight, 23.76f);
    if (NR_SPOT_LIGHTS > 6) spotLightPositions[6] = glm::vec3(22.05f, lightHeight, 26.63f);
    if (NR_SPOT_LIGHTS > 7) spotLightPositions[7] = glm::vec3(18.24f, lightHeight, 38.60f);
    if (NR_SPOT_LIGHTS > 8) spotLightPositions[8] = glm::vec3(6.32f, lightHeight, 33.29f);
    if (NR_SPOT_LIGHTS > 9) spotLightPositions[9] = glm::vec3(1.72f, lightHeight, 49.05f);
    if (NR_SPOT_LIGHTS > 10) spotLightPositions[10] = glm::vec3(8.05f, lightHeight, 51.28f);
    if (NR_SPOT_LIGHTS > 11) spotLightPositions[11] = glm::vec3(27.81f, lightHeight, 58.26f);


    // Load diffuse and normal textures
    textureWall = loadtexture("resources/textures/lab_wall_diffuse.jpg", false);
    textureNormalWall = loadtexture("resources/textures/lab_wall_normal.jpg", false);
    textureFloor = loadtexture("resources/textures/floor_diffuse.jpg", false);
    textureNormalFloor = loadtexture("resources/textures/floor_normal.jpg", false);
    textureCeiling = loadtexture("resources/textures/ceiling_diffuse.jpg", false);
    textureNormalCeiling = loadtexture("resources/textures/ceiling_normal.jpg", false);
    menuTexture = loadtexture("resources/textures/menu.jpg", false);

    setupMenuVAO();
    setupMazeGeometryVAOs();

    menuShader.use();
    menuShader.setInt("menuTexture", 0);

    mazeShader.use();
    mazeShader.setInt("material.diffuse", 0);
    mazeShader.setInt("material.specular", 1); // Assuming you might want a specular map later, or use a default value in shader
    mazeShader.setInt("material.normalMap", 2); // Normal map will be on texture unit 2
    mazeShader.setFloat("material.shininess", mazeShininess);


    if (mainTheme) {
        SoundEngine->play2D(mainTheme, true);
    }

    // Setup GameContext for callbacks to pass multiple objects
    GameContext context = { &sword, &minotaur, &camera };
    glfwSetWindowUserPointer(window, &context);
    // Cursor initially normal for menu, changed to disabled when game starts
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (isGameActive)
        {
            if (!isPlayerDead) {
                sword.Update(deltaTime);
                // Minotaur update
                minotaur.Update(deltaTime, camera.Position);
            }


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

            // Render Floor (diffuse on 0, normal on 2)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureFloor);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureNormalFloor);
            glBindVertexArray(VAO_floor);
            mazeShader.setMat4("model", glm::mat4(1.0f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render Ceiling (diffuse on 0, normal on 2)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureCeiling);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureNormalCeiling);
            glBindVertexArray(VAO_ceiling);
            mazeShader.setMat4("model", glm::mat4(1.0f));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render Walls (diffuse on 0, normal on 2)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureWall);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textureNormalWall);
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
            glBindVertexArray(0); // Unbind VAO after drawing

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
            glBindVertexArray(0); // Unbind VAO after drawing

            // Render Minotaur
            animModelShader.use();
            animModelShader.setMat4("projection", projection);
            animModelShader.setMat4("view", view);
            minotaur.Draw(animModelShader);


            if (!isPlayerDead) {
                sword.Draw(modelShader, camera, projection, view);
            }

            // Render HUD
            glDisable(GL_DEPTH_TEST);

            std::string itemsInfo = "Oggetti: " + std::to_string(itemsFound) + "/3";
            RenderText(itemsInfo.c_str(), 10.0f, SCR_HEIGHT - 40.0f, 0.7f, glm::vec3(1.0, 1.0, 0.0));

            RenderText(("Salute: " + std::to_string((int)playerHealth)).c_str(), 10.0f, SCR_HEIGHT - 60.0f, 0.7f, glm::vec3(0.5, 1.0, 0.5f));
            if (minotaur.health > 0) {
                RenderText(("Minotaur HP: " + std::to_string((int)minotaur.health)).c_str(), SCR_WIDTH - 90.0f, 10.0f, 0.7f, glm::vec3(1.0, 0.3, 0.3));
            }
            else {
                RenderText("Minotaur Sconfitto", SCR_WIDTH - 90.0f, 10.0f, 0.7f, glm::vec3(0.5, 1.0, 0.5f));
            }
            if (isPlayerDead) {
                RenderText("SEI MORTO", SCR_WIDTH / 2.0f - 150.0f, SCR_HEIGHT / 2.0f, 2.0f, glm::vec3(1.0, 0.1, 0.1));
                RenderText("Premi ESC per uscire", SCR_WIDTH / 2.0f - 120.0f, SCR_HEIGHT / 2.0f - 50.0f, 0.7f, glm::vec3(0.8, 0.8, 0.8));
            }
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
    glDeleteTextures(1, &textureNormalWall); // Cleanup normal map textures
    glDeleteTextures(1, &textureNormalFloor); // Cleanup normal map textures
    glDeleteTextures(1, &textureNormalCeiling); // Cleanup normal map textures
    glDeleteVertexArrays(1, &menuVAO);
    glDeleteBuffers(1, &menuVBO);
    glDeleteTextures(1, &menuTexture);

    glfwTerminate();
    return 0;
}


// --- SALUTE GIOCATORE: Funzione per gestire il danno ---
void PlayerTakeDamage(float damage) {
    if (isPlayerDead) return; // Non può subire danni se è già morto

    playerHealth -= damage;
    std::cout << "Player health: " << playerHealth << std::endl; // Messaggio di debug

    if (playerHealth <= 0) {
        playerHealth = 0;
        isPlayerDead = true;
        // Potresti fermare la musica di gioco e far partire un suono di morte
        SoundEngine->stopAllSounds();
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (!isGameActive || isPlayerDead) return;
    GameContext* context = static_cast<GameContext*>(glfwGetWindowUserPointer(window));
    if (!context) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        context->sword->Attack();
        if (attackSound) SoundEngine->play2D(attackSound, false);

        float SWORD_ATTACK_RANGE = 3.5f;
        if (glm::distance(context->camera->Position, context->minotaur->position) < SWORD_ATTACK_RANGE) {
            context->minotaur->TakeDamage(25.0f);
            if (minotaurHitSound) SoundEngine->play2D(minotaurHitSound, false);
            if (context->minotaur->health <= 0 && minotaurDeathSound) SoundEngine->play2D(minotaurDeathSound, false);
        }
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

    GameContext* context = static_cast<GameContext*>(glfwGetWindowUserPointer(window));
    if (!context) return;

    if (!isGameActive) {
        static bool enterPressedLastFrame = false;
        bool enterIsPressed = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

        if (enterIsPressed && !enterPressedLastFrame) {
            isGameActive = true;
            SoundEngine->stopAllSounds();
            if (mainTheme) SoundEngine->play2D(mainTheme, true);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        enterPressedLastFrame = enterIsPressed;
    }
    else {
        if (isPlayerDead) return;

        float desiredHeight = CAMERA_HEIGHT;
        glm::vec3 originalPosition = camera.Position;

        float moveSpeed = 3.5f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime * moveSpeed);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime * moveSpeed);

        glm::vec3 attemptedPosition = camera.Position;
        attemptedPosition.y = desiredHeight;

        if (checkCollision(attemptedPosition, *context->minotaur)) {
            glm::vec3 checkPosNoX = glm::vec3(originalPosition.x, desiredHeight, attemptedPosition.z);
            if (!checkCollision(checkPosNoX, *context->minotaur)) {
                camera.Position.x = originalPosition.x;
            }
            else {
                glm::vec3 checkPosNoZ = glm::vec3(attemptedPosition.x, desiredHeight, originalPosition.z);
                if (!checkCollision(checkPosNoZ, *context->minotaur)) {
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

void loadLevelData(glm::vec3& minotaurSpawnPos) {
    minotaurSpawnPos = glm::vec3(10.0f, 0.0f, 10.0f);

    for (int y = 0; y < MAZE_HEIGHT; ++y) {
        for (int x = 0; x < MAZE_WIDTH; ++x) {
            if (initial_maze_map[y][x] == 1) {
                maze[y][x].wall = true;
            }
            else {
                maze[y][x].wall = false;
                if (initial_maze_map[y][x] == 2) {
                    float worldX = x * CELL_SIZE + CELL_SIZE / 2.0f;
                    float worldZ = y * CELL_SIZE + CELL_SIZE / 2.0f;
                    minotaurSpawnPos = glm::vec3(worldX, 0.0f, worldZ);
                }
            }
        }
    }
}

bool checkCollision(glm::vec3 checkPos, const Minotaur& minotaur) {
    // 1. Collisione con i muri (logica che hai già)
    int gridX = static_cast<int>(floor(checkPos.x / CELL_SIZE));
    int gridZ = static_cast<int>(floor(checkPos.z / CELL_SIZE));

    for (int z = gridZ - 1; z <= gridZ + 1; ++z) {
        for (int x = gridX - 1; x <= gridX + 1; ++x) {
            if (x >= 0 && x < MAZE_WIDTH && z >= 0 && z < MAZE_HEIGHT && maze[z][x].wall) {
                float wallX = (x * CELL_SIZE) + (CELL_SIZE / 2.0f);
                float wallZ = (z * CELL_SIZE) + (CELL_SIZE / 2.0f);

                float closestX = std::max(wallX - CELL_SIZE / 2.0f, std::min(checkPos.x, wallX + CELL_SIZE / 2.0f));
                float closestZ = std::max(wallZ - CELL_SIZE / 2.0f, std::min(checkPos.z, wallZ + CELL_SIZE / 2.0f));

                if (glm::distance(glm::vec2(closestX, closestZ), glm::vec2(checkPos.x, checkPos.z)) < CAMERA_COLLISION_RADIUS) {
                    return true; // Collisione con un muro trovata
                }
            }
        }
    }

    // 2. NUOVA PARTE: Collisione con il minotauro (se è vivo)
    if (minotaur.health > 0) {
        // Calcoliamo la distanza tra il giocatore e il minotauro sul piano XZ
        float distance = glm::distance(glm::vec2(checkPos.x, checkPos.z), glm::vec2(minotaur.position.x, minotaur.position.z));

        // Se la distanza è minore della somma dei loro raggi, c'è una collisione
        if (distance < minotaur.collisionRadius + CAMERA_COLLISION_RADIUS) {
            return true; // Collisione con il minotauro trovata
        }
    }

    return false; // Nessuna collisione
}

void setupMazeGeometryVAOs() {
    // Vertices for a cube, now with Pos, Normal, TexCoords, Tangent, Bitangent
    // Each vertex now has 3+3+2+3+3 = 14 floats
    float cubeVertices[] = {
        // positions            // normals           // texcoords // tangents            // bitangents
        // Back face (-Z)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // BL
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // BR
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // TR
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // TR
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // TL
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // BL

        // Front face (+Z)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // BL
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // BR
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // TR
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // TR
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // TL
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f, // BL

        // Left face (-X)
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f, // TL
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f, // BL
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f, // BR
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f, // BR
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f, // TR
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f, // TL

        // Right face (+X)
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // TR
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // TL
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // BL
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // BL
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // BR
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, // TR


         // Bottom face (-Y)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // BL
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // BR
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // TR
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // TR
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // TL
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f, // BL

        // Top face (+Y)
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f, // BL
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f, // BR
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f, // TR
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f, // TR
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f, // TL
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f  // BL
    };
    glGenBuffers(1, &VBO_cube_lit);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO_walls);
    glBindVertexArray(VAO_walls);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // Bitangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);

    float mazeW = (float)MAZE_WIDTH * CELL_SIZE;
    float mazeD = (float)MAZE_HEIGHT * CELL_SIZE;
    float textureRepeatX = mazeW / CELL_SIZE;
    float textureRepeatZ = mazeD / CELL_SIZE;


    // Floor vertices (Positions, Normals, TexCoords, Tangent, Bitangent)
    // Normal for floor is (0, 1, 0) in model space (upwards)
    // Tangent for floor: (1, 0, 0)
    // Bitangent for floor: (0, 0, 1)
    float floorVertices[] = {
        // positions            // normals           // texcoords          // tangents        // bitangents
        mazeW, 0.0f, mazeD,       0.0f, 1.0f, 0.0f,   textureRepeatX, textureRepeatZ,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, // TR
        mazeW, 0.0f, 0.0f,        0.0f, 1.0f, 0.0f,   textureRepeatX, 0.0f,            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, // BR
        0.0f,  0.0f, 0.0f,        0.0f, 1.0f, 0.0f,   0.0f,           0.0f,            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, // BL
        0.0f,  0.0f, mazeD,       0.0f, 1.0f, 0.0f,   0.0f,           textureRepeatZ,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f  // TL
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
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // Bitangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);

    // Ceiling vertices (Positions, Normals, TexCoords, Tangent, Bitangent)
    // Normal for ceiling is (0, -1, 0) in model space (downwards)
    // Tangent for ceiling: (1, 0, 0) (along X)
    // Bitangent for ceiling: (0, 0, -1) (along -Z, because normal is -Y and tangent is X: X cross -Y = -Z)
    float ceilingVertices[] = {
        // positions                 // normals           // texcoords          // tangents        // bitangents
        mazeW, WALL_HEIGHT, mazeD,   0.0f, -1.0f, 0.0f,   textureRepeatX, textureRepeatZ,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, // TR
        0.0f,  WALL_HEIGHT, mazeD,   0.0f, -1.0f, 0.0f,   0.0f,           textureRepeatZ,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, // TL
        0.0f,  WALL_HEIGHT, 0.0f,    0.0f, -1.0f, 0.0f,   0.0f,           0.0f,            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f, // BL
        mazeW, WALL_HEIGHT, 0.0f,    0.0f, -1.0f, 0.0f,   textureRepeatX, 0.0f,            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f  // BR
    };
    unsigned int ceilingIndices[] = { 0, 1, 3, 1, 2, 3 }; // Adjusted indices for a different winding order if needed
    unsigned int VBO_ceiling, EBO_ceiling;
    glGenVertexArrays(1, &VAO_ceiling);
    glGenBuffers(1, &VBO_ceiling);
    glGenBuffers(1, &EBO_ceiling);
    glBindVertexArray(VAO_ceiling);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ceiling);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ceilingVertices), ceilingVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ceiling);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ceilingIndices), ceilingIndices, GL_STATIC_DRAW);
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Tangent
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // Bitangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);

    glGenVertexArrays(1, &VAO_lamp);
    glBindVertexArray(VAO_lamp);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0); // Only position for lamp (the rest is not needed for lamp shader)
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void setupWindowVAO() {
    float transparentVertices[] = {
        // positions          // texCoords
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

void setupMenuVAO() {
    float menuVertices[] = {
        // positions  // texCoords
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