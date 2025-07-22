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

#include <queue>

#include <irrklang/irrKlang.h>

#include "render_text.h"

#include "cell.h"

#include "config.h"

#include "wallSconce.h" 
#include "ParticleEmitter.h" // sistema particellare



#pragma comment(lib, "irrKlang.lib")



#include "shader_m.h"

#include "camera.h"

#include "sword.h"

#include "animator.h"

#include "minotaur.h"

#include "chest.h" 



#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "timer.h"

#include "dungeon_room.h"

#include "enemy.h"



using namespace irrklang;



// Struct per passare dati alle callback

struct GameContext {

    Sword* sword;

    Minotaur* minotaur;

    Camera* camera;

    //std::vector<Chest>* chests; // <-- NUOVO PUNTATORE

    std::vector<DungeonRoom>* dungeonRooms; //nuovo

};



// --- STATO DEL GIOCO ---

enum class GameState {

    MENU,

    PLAYING,

    VICTORY,

    GAME_OVER

};



GameState currentState = GameState::MENU;



//--- Variabili Globali Audio ---

ISoundEngine* SoundEngine = createIrrKlangDevice();

ISoundSource* mainTheme = SoundEngine->addSoundSourceFromFile("resources/main.mp3");

ISoundSource* attackSound = SoundEngine->addSoundSourceFromFile("resources/sword_swing.mp3");

ISoundSource* minotaurHitSound = SoundEngine->addSoundSourceFromFile("resources/hit.mp3");

ISoundSource* minotaurDeathSound = SoundEngine->addSoundSourceFromFile("resources/death.mp3");

ISoundSource* playerHurtSound = SoundEngine->addSoundSourceFromFile("resources/hurt.mp3");



// --- Variabili Globali di Gioco ---

float playerHealth = 100.0f;

const float PLAYER_MAX_HEALTH = 100.0f;

float playerAttackDamage = 50.0f;





// --- NUOVO TIMER PER MESSAGGIO VITTORIA ---

Timer victoryMessageTimer(5.0f); // Il messaggio dura 5 secondi

Timer damageEffectTimer(0.5f); // L'effetto dura mezzo secondo

Timer cameraShakeTimer(0.3f); // <-- NUOVO TIMER
unsigned int damageOverlayTexture;



std::vector<DungeonRoom> dungeonRooms;

Model* enemyModel_ptr = nullptr;



// --- Variabili per le Casse ---

std::vector<Chest> chests;

Model* chestModel_ptr = nullptr;

Timer powerUpMessageTimer(4.0f);

std::string lastPowerUpMessage = "";


//Variabili per le fiaccole
std::vector<WallSconce> wallSconces;
Model* wallSconceModel_ptr = nullptr;

//Variabili per il sistema particellare
std::vector<ParticleEmitter> fireEmitters;

//Variabili per illuminazione
const int MAX_POINT_LIGHTS = 10;
glm::vec3 pointLightPositions[MAX_POINT_LIGHTS];
int activePointLights = 0;



// --- STAMINA: Nuove variabili ---

float playerStamina = 100.0f;

const float PLAYER_MAX_STAMINA = 100.0f;

const float SPRINT_DRAIN_RATE = 25.0f; // Punti al secondo

const float STAMINA_REGEN_RATE = 15.0f; // Punti al secondo


// Muro = 1, Minotauro = 2, Uscita = 3, Casse = 4
const int initial_maze_map[MAP_SIZE_ROWS][MAP_SIZE_COLS] = {

    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    { 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1 },

    { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },

    { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },

    { 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 },

    { 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1 },

    { 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 4, 0, 0, 0, 1, 1 },

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

    { 1, 0, 0, 4, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 },

    { 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },

    { 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },

    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 4, 0, 0, 1 },

    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 1 },

    { 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 },

    { 1, 1, 1, 1, 1, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } 

};



std::vector<std::vector<Cell>> maze(MAZE_HEIGHT, std::vector<Cell>(MAZE_WIDTH));

Camera camera(glm::vec3(0.0f, CAMERA_HEIGHT, 0.0f));

float lastX = SCR_WIDTH / 2.0f;

float lastY = SCR_HEIGHT / 2.0f;

bool firstMouse = true;

float deltaTime = 0.0f;

float lastFrame = 0.0f;



unsigned int VBO_cube_lit, VAO_walls, VAO_floor, VAO_ceiling;

unsigned int VAO_lamp, menuVAO, menuVBO;

unsigned int textureWall, textureFloor, textureCeiling, menuTexture, endMenuTexture;

unsigned int textureNormalWall, textureNormalFloor, textureNormalCeiling;



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





// --- VARIABILI PER IL FILO DI ARIANNA ---

Shader* hintShader;

unsigned int hintVAO, hintVBO;

std::vector<glm::vec3> hintPath;

bool showHint = false;

float hintTimer = 0.0f;

const float HINT_DURATION = 5.0f;

bool ariadneThreadUnlocked = false;  // True dopo aver sconfitto il Minotauro

Timer unlockMessageTimer(7.0f);      // Timer per il messaggio di sblocco



// Struct AStarNode (necessaria per FindPathForHint)

struct AStarNode {

    int total_cost;

    glm::vec2 position;



    bool operator>(const AStarNode& other) const {

        return total_cost > other.total_cost;

    }

};



// Dichiarazioni funzioni

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow* window);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void loadLevelData(glm::vec3& minotaurSpawnPos);

bool checkWallCollision(glm::vec3 checkPos);

bool checkMinotaurCollision(glm::vec3 checkPos, const Minotaur& minotaur);

bool checkCollision(glm::vec3 checkPos, const Minotaur& minotaur); //nuovo

void setupMazeGeometryVAOs();

void setupMenuVAO();

void ResetGame(Camera& cam, Minotaur& minotaur);

unsigned int loadtexture(const std::string& path, bool clampToEdge = false);

void PlayerTakeDamage(float damage);

inline unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma);

void FindPathForHint(glm::vec2 start, glm::vec2 target, std::vector<glm::vec3>& path);



int main() {

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

#endif



    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Theseus' Labyrinth", NULL, NULL);

    if (window == NULL) {

        std::cout << "Failed to create GLFW window" << std::endl;

        glfwTerminate();

        return -1;

    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetScrollCallback(window, scroll_callback);

    glfwSetMouseButtonCallback(window, mouse_button_callback);



    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

        std::cout << "Failed to initialize GLAD" << std::endl;

        return -1;

    }



    initRenderText(SCR_WIDTH, SCR_HEIGHT);

    stbi_set_flip_vertically_on_load(false);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    Shader modelShader("model.vs", "model.fs");

    Shader mazeShader("spot_light.vs", "spot_light.fs");

    Shader lampShader("lamp.vs", "lamp.fs");

    Shader menuShader("menu.vs", "menu.fs");

    Shader animModelShader("anim_model.vs", "anim_model.fs");

    hintShader = new Shader("line.vs", "line.fs");



    Sword sword("resources/sword/sword.obj");

    Model minotaurModel("resources/minotaur/minotauro.glb");

    minotaurModel.LoadAnimation("idle", "resources/minotaur/idle.glb");

    minotaurModel.LoadAnimation("walk", "resources/minotaur/walk.glb");

    minotaurModel.LoadAnimation("get_hit", "resources/minotaur/get_hit.glb");

    minotaurModel.LoadAnimation("death", "resources/minotaur/death.glb");

    minotaurModel.LoadAnimation("attack", "resources/minotaur/attack.glb");



    // Carica il modello della cassa

    chestModel_ptr = new Model("resources/chest/chest.glb");



    // --- AGGIUNGI QUESTA RIGA QUI ---

    enemyModel_ptr = new Model("resources/enemy/golem.glb");

    //Modello della fiaccola
    wallSconceModel_ptr = new Model("resources/torch/torch.glb");


    // --- BLOCCO DA SOSTITUIRE ---
    // Definisci una struttura per i dati della torcia, ora con l'offset
    struct TorchData {
        glm::vec3 position;
        float     rotation;
        float     scale;
        glm::vec3 flameOffset; // Offset locale per la fiamma
    };

    // Crea la tua lista di torce, specificando l'offset per ciascuna
    std::vector<TorchData> torchPositions = {
        { glm::vec3(1.9f, 1.5f, 5.0f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(1.9f, 1.5f, 18.90f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(17.7564f, 1.5f, 25.22f), -90.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(11.2421f, 1.5f, 28.73f), 90.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(10.3171f, 1.5f, 17.787f), 90.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(20.90f, 1.5f, 7.1f), 90.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(19.90f, 1.5, 12.50f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(26.9f, 1.5f, 30.75f), 180.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(21.75f, 1.5f, 41.43f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(7.3f, 1.5f, 41.66f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(1.85f, 1.5f, 29.89f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(1.83f, 1.5, 54.14), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(19.85f, 1.5f, 60.41f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(9.05f, 1.5f, 64.53f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(25.25f, 1.5f, 67.54f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
        { glm::vec3(25.25f, 1.5f, 53.23f), 0.0f, 2.0f, glm::vec3(0.25f, 0.3f, 0.0f) },
    };

    activePointLights = 0;

    if (wallSconceModel_ptr) {
        for (const auto& data : torchPositions) {
            // Passa l'offset al costruttore della torcia
            wallSconces.emplace_back(*wallSconceModel_ptr, data.position, data.rotation, data.scale, data.flameOffset);
            fireEmitters.emplace_back(500); // Riduci il numero di particelle per performance
        }
    }



    glm::vec3 minotaurStartPosition;

    loadLevelData(minotaurStartPosition);



    camera.Position = glm::vec3(1 * CELL_SIZE + CELL_SIZE / 2.0f, CAMERA_HEIGHT, 1 * CELL_SIZE + CELL_SIZE / 2.0f);

    Minotaur minotaur(minotaurModel, minotaurStartPosition, maze);


    float lightHeight = WALL_HEIGHT - 0.1f;
    //spotLightPositions[0] = glm::vec3(1.5f * CELL_SIZE, lightHeight, 1.5f * CELL_SIZE);
    //if (NR_SPOT_LIGHTS > 1) spotLightPositions[1] = glm::vec3(2.03918f, lightHeight, 2.43882f);
    if (NR_SPOT_LIGHTS > 1) spotLightPositions[1] = glm::vec3(2.07f, 2.6f, 18.91f);
    //if (NR_SPOT_LIGHTS > 2) spotLightPositions[2] = glm::vec3(2.03109f, lightHeight, 23.1429f);
    if (NR_SPOT_LIGHTS > 3) spotLightPositions[3] = glm::vec3(1.9f, 2.6f, 5.0f);
    if (NR_SPOT_LIGHTS > 4) spotLightPositions[4] = glm::vec3(17.7564f, 2.6f, 25.4229f);
    if (NR_SPOT_LIGHTS > 5) spotLightPositions[5] = glm::vec3(11.2421f, 2.6f, 28.5234f);
    if (NR_SPOT_LIGHTS > 6) spotLightPositions[6] = glm::vec3(10.3171f, 2.6f, 17.787f);
    if (NR_SPOT_LIGHTS > 7) spotLightPositions[7] = glm::vec3(20.90f, 2.6f, 7.1f);
    if (NR_SPOT_LIGHTS > 8) spotLightPositions[8] = glm::vec3(19.90f, 2.6, 12.50f);
    if (NR_SPOT_LIGHTS > 9) spotLightPositions[9] = glm::vec3(26.7f, 2.6f, 30.75f);
    if (NR_SPOT_LIGHTS > 10) spotLightPositions[10] = glm::vec3(21.80f, 2.6f, 41.43f);
    if (NR_SPOT_LIGHTS > 11) spotLightPositions[11] = glm::vec3(7.3f, 2.6f, 41.66f);
    if (NR_SPOT_LIGHTS > 12) spotLightPositions[12] = glm::vec3(1.85f, 2.6f, 29.89f);
    if (NR_SPOT_LIGHTS > 13) spotLightPositions[13] = glm::vec3(1.83f, 2.6f, 54.14);
    if (NR_SPOT_LIGHTS > 14) spotLightPositions[14] = glm::vec3(19.85f, 2.6f, 60.41f);
    if (NR_SPOT_LIGHTS > 15) spotLightPositions[15] = glm::vec3(9.15f, 2.6f, 64.53f);
    if (NR_SPOT_LIGHTS > 16) spotLightPositions[16] = glm::vec3(25.32f, 2.6f, 67.54f);
    if (NR_SPOT_LIGHTS > 17) spotLightPositions[17] = glm::vec3(25.32f, 2.6f, 53.23f);



    //float lightHeight = WALL_HEIGHT - 0.1f;

    ////spotLightPositions[0] = glm::vec3(1.5f * CELL_SIZE, lightHeight, 1.5f * CELL_SIZE);

    //if (NR_SPOT_LIGHTS > 1) spotLightPositions[1] = glm::vec3(2.03918f, lightHeight, 2.43882f);

    //if (NR_SPOT_LIGHTS > 2) spotLightPositions[2] = glm::vec3(2.03109f, lightHeight, 23.1429f);

    //if (NR_SPOT_LIGHTS > 3) spotLightPositions[3] = glm::vec3(12.167f, lightHeight, 14.8177f);

    //if (NR_SPOT_LIGHTS > 4) spotLightPositions[4] = glm::vec3(17.7564f, lightHeight, 25.4229f);

    //if (NR_SPOT_LIGHTS > 5) spotLightPositions[5] = glm::vec3(11.2421f, lightHeight, 28.5234f);

    //if (NR_SPOT_LIGHTS > 6) spotLightPositions[6] = glm::vec3(9.20282f, lightHeight, 3.86636f);

    //if (NR_SPOT_LIGHTS > 7) spotLightPositions[7] = glm::vec3(28.7836f, lightHeight, 2.01158f);

    //if (NR_SPOT_LIGHTS > 8) spotLightPositions[8] = glm::vec3(25.5885f, lightHeight, 12.5597f);

    //if (NR_SPOT_LIGHTS > 9) spotLightPositions[9] = glm::vec3(30.0438f, lightHeight, 41.6945f);

    //if (NR_SPOT_LIGHTS > 10) spotLightPositions[10] = glm::vec3(4.65741f, lightHeight, 31.8666f);

    //if (NR_SPOT_LIGHTS > 11) spotLightPositions[11] = glm::vec3(17.7664f, lightHeight, 42.9045f);

    //if (NR_SPOT_LIGHTS > 12) spotLightPositions[12] = glm::vec3(7.44922f, lightHeight, 39.807f);

    //if (NR_SPOT_LIGHTS > 13) spotLightPositions[13] = glm::vec3(2.04897f, lightHeight, 59.1026f);

    //if (NR_SPOT_LIGHTS > 14) spotLightPositions[14] = glm::vec3(23.1384f, lightHeight, 50.7383f);

    //if (NR_SPOT_LIGHTS > 15) spotLightPositions[15] = glm::vec3(9.20907f, lightHeight, 61.4568f);

    //if (NR_SPOT_LIGHTS > 16) spotLightPositions[16] = glm::vec3(30.5345f, lightHeight, 60.612f);



    textureWall = loadtexture("resources/textures/lab_wall_diffuse.jpg");

    textureNormalWall = loadtexture("resources/textures/lab_wall_normal.jpg");

    textureFloor = loadtexture("resources/textures/floor_diffuse.jpg");

    textureNormalFloor = loadtexture("resources/textures/floor_normal.jpg");

    textureCeiling = loadtexture("resources/textures/ceiling_diffuse.jpg");

    textureNormalCeiling = loadtexture("resources/textures/ceiling_normal.jpg");

    menuTexture = loadtexture("resources/textures/menu.jpg");

    endMenuTexture = loadtexture("resources/textures/endmenu.jpg");
    // AGGIUNGI QUESTA RIGA
    damageOverlayTexture = loadtexture("resources/textures/damage_overlay.png", true);



    setupMenuVAO();

    setupMazeGeometryVAOs();



    // Setup VAO e VBO per il filo di Arianna

    glGenVertexArrays(1, &hintVAO);

    glGenBuffers(1, &hintVBO);

    glBindVertexArray(hintVAO);

    glBindBuffer(GL_ARRAY_BUFFER, hintVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MAP_SIZE_ROWS * MAP_SIZE_COLS, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);



    menuShader.use();

    menuShader.setInt("menuTexture", 0);



    mazeShader.use();

    mazeShader.setInt("material.diffuse", 0);

    mazeShader.setInt("material.specular", 1);

    mazeShader.setInt("material.normalMap", 2);

    mazeShader.setFloat("material.shininess", mazeShininess);



    if (mainTheme) SoundEngine->play2D(mainTheme, true);

    // Inizializza il GameContext con i puntatori corretti

    GameContext context = { &sword, &minotaur, &camera, &dungeonRooms };

    glfwSetWindowUserPointer(window, &context);



    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);



    // --- GAME LOOP ---

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());

        deltaTime = currentFrame - lastFrame;

        lastFrame = currentFrame;

        processInput(window);



        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        glm::mat4 view = camera.GetViewMatrix();


        // Calcola l'intensità della luce pulsante usando il tempo di gioco
        float time = glfwGetTime();
        // La funzione sin() oscilla tra -1 e 1. La mappiamo nell'intervallo [0.5, 1.0]
        // in modo che la luce si attenui al 50% e torni al 100%, senza mai spegnersi.
        // Il valore 5.0f controlla la velocità del tremolio. Aumentalo per un effetto più rapido.
        float flickerIntensity = 0.75f + sin(time * 20.0f) * 0.25f;

        // Calcola il colore diffuso aggiornato in base all'intensità
        glm::vec3 flickeringDiffuse = spotLightDiffuse * flickerIntensity;

        // --- LOGICA CAMERA SHAKE ---
        if (cameraShakeTimer.IsActive()) {
            float shakeAmount = 0.08f * (cameraShakeTimer.GetTime() / 0.3f); // 0.3f è la durata
            float offsetX = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            float offsetY = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            glm::mat4 shakeTransform = glm::translate(glm::mat4(1.0f), glm::vec3(offsetX, offsetY, 0.0f) * shakeAmount);
            view = shakeTransform * view; // Applica il tremore alla matrice di vista
        }



        switch (currentState) {

        case GameState::MENU: {

            glDisable(GL_DEPTH_TEST);

            menuShader.use();

            glActiveTexture(GL_TEXTURE0);

            glBindTexture(GL_TEXTURE_2D, menuTexture);

            glBindVertexArray(menuVAO);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_DEPTH_TEST);

            break;

        }

        case GameState::PLAYING:

        case GameState::GAME_OVER: {

            if (currentState == GameState::PLAYING) {

                sword.Update(deltaTime);

                minotaur.Update(deltaTime, camera.Position);

                for (auto& room : dungeonRooms) {

                    room.Update(deltaTime, camera.Position);

                }

                if (showHint) {

                    hintTimer -= deltaTime;

                    if (hintTimer <= 0.0f) {

                        showHint = false;

                        hintTimer = 0.0f;

                    }

                }

                // --- AGGIORNA IL NUOVO TIMER ---

                victoryMessageTimer.Update(deltaTime);

                // --- AGGIUNTA CASSE: Aggiorna il timer del messaggio power-up ---

                powerUpMessageTimer.Update(deltaTime);

                for (size_t i = 0; i < wallSconces.size(); ++i) {
                    WallSconce& sconce = wallSconces[i];

                    // 1. Crea la matrice di rotazione per la torcia corrente
                    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(sconce.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

                    // 2. Ruota il vettore di offset locale della fiamma
                    glm::vec3 rotatedOffset = glm::vec3(rotationMatrix * glm::vec4(sconce.flameOffset, 1.0f));

                    // 3. Calcola la posizione finale della fiamma nel mondo
                    glm::vec3 firePosition = sconce.position + rotatedOffset;

                    // 4. Aggiorna l'emettitore con la posizione corretta
                    fireEmitters[i].Update(deltaTime, firePosition, 30); // Genera poche particelle per frame
                }

                unlockMessageTimer.Update(deltaTime);
                damageEffectTimer.Update(deltaTime); // <-- AGGIORNAMENTO NUOVO TIMER
                cameraShakeTimer.Update(deltaTime); // <-- AGGIORNA IL NUOVO TIMER

            }



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
                mazeShader.setVec3(lightUni + ".diffuse", flickeringDiffuse);
                mazeShader.setVec3(lightUni + ".specular", spotLightSpecular);
                mazeShader.setFloat(lightUni + ".constant", spotLightConstant);
                mazeShader.setFloat(lightUni + ".linear", spotLightLinear);
                mazeShader.setFloat(lightUni + ".quadratic", spotLightQuadratic);
            }

            //Illuminazione particellare
            mazeShader.setInt("activePointLights", activePointLights);
            for (int i = 0; i < activePointLights; i++) {
                std::string name = "pointLights[" + std::to_string(i) + "]";
                mazeShader.setVec3(name + ".position", pointLightPositions[i]);
                mazeShader.setVec3(name + ".ambient", 0.05f, 0.05f, 0.05f);
                mazeShader.setVec3(name + ".diffuse", 0.8f, 0.6f, 0.2f);
                mazeShader.setVec3(name + ".specular", 1.0f, 1.0f, 1.0f);
                mazeShader.setFloat(name + ".constant", 1.0f);
                mazeShader.setFloat(name + ".linear", 0.09f);
                mazeShader.setFloat(name + ".quadratic", 0.032f);
            }



            glActiveTexture(GL_TEXTURE0);

            glBindTexture(GL_TEXTURE_2D, textureFloor);

            glActiveTexture(GL_TEXTURE2);

            glBindTexture(GL_TEXTURE_2D, textureNormalFloor);

            glBindVertexArray(VAO_floor);

            mazeShader.setMat4("model", glm::mat4(1.0f));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);







            if (showHint && !hintPath.empty()) {

                hintShader->use();

                hintShader->setMat4("projection", projection);

                hintShader->setMat4("view", view);

                hintShader->setMat4("model", glm::mat4(1.0f));

                glBindVertexArray(hintVAO);

                glDrawArrays(GL_LINE_STRIP, 0, hintPath.size());

                glBindVertexArray(0);

            }



            glActiveTexture(GL_TEXTURE0);

            glBindTexture(GL_TEXTURE_2D, textureCeiling);

            glActiveTexture(GL_TEXTURE2);

            glBindTexture(GL_TEXTURE_2D, textureNormalCeiling);

            glBindVertexArray(VAO_ceiling);

            mazeShader.setMat4("model", glm::mat4(1.0f));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



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



            if (minotaur.currentState != Minotaur::State::FINISHED) {

                animModelShader.use();

                animModelShader.setMat4("projection", projection);

                animModelShader.setMat4("view", view);

                //animModelShader.setVec3("viewPos", camera.Position);

                minotaur.Draw(animModelShader);

            }



            // --- AGGIUNTA CASSE: Render delle casse ---

            modelShader.use();

            modelShader.setMat4("projection", projection);

            modelShader.setMat4("view", view);



            // --- CORREZIONE: Render delle Casse e dei loro Nemici ---

            for (auto& room : dungeonRooms) {

                // Disegna la cassa della stanza

                modelShader.use(); // Usa lo shader per modelli statici

                modelShader.setMat4("projection", projection);

                modelShader.setMat4("view", view);

                modelShader.setVec3("viewPos", camera.Position);

                room.chest.Draw(modelShader);



                // Disegna i nemici della stanza

                modelShader.use(); // Usa lo shader per modelli statici (o animModelShader se sono animati)

                modelShader.setMat4("projection", projection);

                modelShader.setMat4("view", view);

                modelShader.setVec3("viewPos", camera.Position);

                for (auto& enemy : room.enemies) {

                    enemy.Draw(modelShader);

                }

            }



            /*lampShader.use();

            lampShader.setMat4("projection", projection);

            lampShader.setMat4("view", view);

            glBindVertexArray(VAO_lamp);

            for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {

                glm::mat4 model = glm::mat4(1.0f);

                model = glm::translate(model, spotLightPositions[i]);

                model = glm::scale(model, glm::vec3(0.15f));

                lampShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);

            }*/







            if (currentState == GameState::PLAYING) {

                sword.Draw(modelShader, camera, projection, view);

            }



            glDisable(GL_DEPTH_TEST);

            RenderText(("Salute: " + std::to_string(static_cast<int>(playerHealth))).c_str(), 10.0f, SCR_HEIGHT - 60.0f, 0.7f, glm::vec3(0.5, 1.0, 0.5f));


            if (unlockMessageTimer.IsActive()) {
                RenderText("HAI SBLOCCATO IL FILO CHE CONDUCE ALLA LIBERTA'. PREMI H PER UTILIZZARLO",
                    SCR_WIDTH / 2.0f - 220.0f, SCR_HEIGHT / 2.0f - 50.0f, 0.7f, glm::vec3(1.0, 0.84, 0.0)); // Colore oro
            }


            // --- NUOVO: RENDER EFFETTO DANNO ---
        // Lo disegniamo dopo la scena 3D ma prima dell'HUD
            if (damageEffectTimer.IsActive()) {
                glDisable(GL_DEPTH_TEST); // Disabilita il test di profondità per disegnarlo sopra a tutto
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                menuShader.use(); // Riutilizziamo lo shader del menu che disegna una texture a schermo intero

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, damageOverlayTexture);

                glBindVertexArray(menuVAO); // Riutilizziamo il VAO del menu che è un quad a schermo intero
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glEnable(GL_DEPTH_TEST); // Riabilita il test di profondità
            }


            // --- STAMINA: Disegna la barra della stamina ---

            std::string staminaBar = "Stamina: [";

            int barWidth = 20;

            int filledWidth = static_cast<int>((playerStamina / PLAYER_MAX_STAMINA) * barWidth);

            for (int i = 0; i < barWidth; ++i) {

                staminaBar += (i < filledWidth) ? '|' : ' ';

            }

            staminaBar += "]";

            RenderText(staminaBar.c_str(), 10.0f, SCR_HEIGHT - 75.0f, 0.6f, glm::vec3(0.9, 0.9, 0.2));



            // --- BARRA VITA MINOTAURO ---

            // Mostra la barra solo se il Minotauro è nel raggio di avviso e di fronte al giocatore

            // Logica per Barra Vita / Messaggio Sconfitta Minotauro

            if (victoryMessageTimer.IsActive()) {

                RenderText("Minotauro Sconfitto", SCR_WIDTH / 2.0f - 120.0f, SCR_HEIGHT - 50.0f, 0.8f, glm::vec3(0.5, 1.0, 0.5f));

            }



            else if (minotaur.health > 0) {



                float distanceToMino = glm::distance(camera.Position, minotaur.position);



                glm::vec3 toMinoDir = glm::normalize(minotaur.position - camera.Position);



                float dotProduct = glm::dot(camera.Front, toMinoDir);



                if (distanceToMino < minotaur.NOTICE_RANGE && dotProduct > 0.3f) {



                    std::string minoHealthBar = "Minotauro: [";



                    int minoBarWidth = 25;



                    int minoFilledWidth = static_cast<int>((minotaur.health / 100.0f) * minoBarWidth);



                    for (int i = 0; i < minoBarWidth; ++i) {



                        minoHealthBar += (i < minoFilledWidth) ? '#' : ' ';



                    }



                    minoHealthBar += "]";



                    RenderText(minoHealthBar.c_str(), 800.0f, SCR_HEIGHT - 90.0f, 0.7f, glm::vec3(1.0, 0.3, 0.3));

                }



            }





            bool canOpenChest = false;

            bool chestIsLocked = false;

            if (currentState == GameState::PLAYING) {

                for (const auto& room : dungeonRooms) {

                    if (!room.chest.isCollected && glm::distance(camera.Position, room.chest.position) < 2.5f) {

                        canOpenChest = true;

                        if (room.chest.isLocked) {

                            chestIsLocked = true;

                        }

                        break;

                    }

                }

            }

            if (canOpenChest) {

                if (chestIsLocked) {

                    RenderText("La cassa e' bloccata, sconfiggi i guardiani!", SCR_WIDTH / 2.0f - 220.0f, SCR_HEIGHT / 2.0f - 50.0f, 0.7f, glm::vec3(1.0, 0.5, 0.0));

                }

                else {

                    RenderText("Apri la cassa (E)", SCR_WIDTH / 2.0f - 100.0f, SCR_HEIGHT / 2.0f - 50.0f, 0.7f, glm::vec3(1.0f));

                }

            }



            if (powerUpMessageTimer.IsActive()) {

                RenderText(lastPowerUpMessage.c_str(), SCR_WIDTH / 2.0f - 220.0f, SCR_HEIGHT / 2.0f - 50.0f, 0.7f, glm::vec3(1.0, 0.5, 0.0));

            }



            // --- LOGICA PER BARRE VITA NEMICI VISIBILI ---

            float healthBarYOffset = SCR_HEIGHT - 40.0f;

            int visibleEnemyCount = 0;



            // Prima, identifica quali nemici sono visibili

            std::vector<const Enemy*> visibleEnemies;

            if (currentState == GameState::PLAYING) {

                for (const auto& room : dungeonRooms) {

                    if (room.state == DungeonRoom::RoomState::ACTIVE) {

                        for (const auto& enemy : room.enemies) {

                            if (enemy.health > 0) {

                                float distanceToEnemy = glm::distance(camera.Position, enemy.position);

                                glm::vec3 toEnemyDir = glm::normalize(enemy.position - camera.Position);

                                float dotProduct = glm::dot(camera.Front, toEnemyDir);



                                // Aggiungi alla lista solo se è vicino e di fronte

                                if (distanceToEnemy < enemy.NOTICE_RANGE && dotProduct > 0.4f) {

                                    visibleEnemies.push_back(&enemy);

                                }

                            }

                        }

                    }

                }

            }



            // Ora, disegna una barra per ogni nemico nella lista dei visibili

            if (!visibleEnemies.empty()) {

                RenderText("Guardiani:", SCR_WIDTH - 250.0f, healthBarYOffset, 0.6f, glm::vec3(1.0, 0.5, 0.5));

                healthBarYOffset -= 25.0f;



                for (const auto* enemyPtr : visibleEnemies) {

                    std::string enemyHealthBar = "[";

                    int barWidth = 15;

                    int filledWidth = static_cast<int>((enemyPtr->health / 50.0f) * barWidth);

                    for (int i = 0; i < barWidth; ++i) {

                        enemyHealthBar += (i < filledWidth) ? '#' : ' ';

                    }

                    enemyHealthBar += "]";



                    RenderText(enemyHealthBar.c_str(), SCR_WIDTH - 250.0f, healthBarYOffset, 0.5f, glm::vec3(0.9, 0.9, 0.9));

                    healthBarYOffset -= 20.0f;

                }

            }







            if (currentState == GameState::GAME_OVER) {

                RenderText("SEI MORTO", SCR_WIDTH / 2.0f - 150.0f, SCR_HEIGHT / 2.0f, 2.0f, glm::vec3(1.0, 0.1, 0.1));

                RenderText("Premi INVIO per ricominciare o ESC per uscire", SCR_WIDTH / 2.0f - 220.0f, SCR_HEIGHT / 2.0f - 50.0f, 0.7f, glm::vec3(0.8, 0.8, 0.8));

            }

            glEnable(GL_DEPTH_TEST);

            break;

        }

        case GameState::VICTORY: {

            glDisable(GL_DEPTH_TEST);

            menuShader.use();

            glActiveTexture(GL_TEXTURE0);

            glBindTexture(GL_TEXTURE_2D, endMenuTexture);

            glBindVertexArray(menuVAO);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_DEPTH_TEST);

            break;

        }

        }

        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setFloat("shininess", 32.0f);

        // Passa le luci allo shader (come già fai)
        for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
            // ... il tuo ciclo for per le luci ...
            std::string lightUni = "spotLights[" + std::to_string(i) + "]";
            modelShader.setVec3(lightUni + ".position", spotLightPositions[i]);
            modelShader.setVec3(lightUni + ".direction", spotLightDirection);
            modelShader.setFloat(lightUni + ".cutOff", spotLightCutOff);
            modelShader.setFloat(lightUni + ".outerCutOff", spotLightOuterCutOff);
            modelShader.setVec3(lightUni + ".ambient", spotLightAmbient);
            modelShader.setVec3(lightUni + ".diffuse", spotLightDiffuse);
            modelShader.setVec3(lightUni + ".specular", spotLightSpecular);
            modelShader.setFloat(lightUni + ".constant", spotLightConstant);
            modelShader.setFloat(lightUni + ".linear", spotLightLinear);
            modelShader.setFloat(lightUni + ".quadratic", spotLightQuadratic);
        }

        // Disegna le fiaccole
        for (auto& sconce : wallSconces) {
            sconce.Draw(modelShader);
        }

        // Disegna le particelle del fuoco
        for (auto& emitter : fireEmitters) {
            emitter.Draw(view, projection);
        }



        glfwSwapBuffers(window);

        glfwPollEvents();

    }



    SoundEngine->drop();

    delete hintShader;

    glDeleteVertexArrays(1, &VAO_walls);

    glDeleteVertexArrays(1, &VAO_floor);

    glDeleteVertexArrays(1, &VAO_ceiling);

    glDeleteVertexArrays(1, &VAO_lamp);

    glDeleteVertexArrays(1, &menuVAO);

    glDeleteVertexArrays(1, &hintVAO);

    glDeleteBuffers(1, &VBO_cube_lit);

    glDeleteBuffers(1, &menuVBO);

    glDeleteBuffers(1, &hintVBO);

    glDeleteTextures(1, &textureWall);

    glDeleteTextures(1, &textureFloor);

    glDeleteTextures(1, &textureCeiling);

    glDeleteTextures(1, &textureNormalWall);

    glDeleteTextures(1, &textureNormalFloor);

    glDeleteTextures(1, &textureNormalCeiling);

    glDeleteTextures(1, &menuTexture);

    glDeleteTextures(1, &endMenuTexture);



    glfwTerminate();

    return 0;

}



void framebuffer_size_callback(GLFWwindow* window, int width, int height) {

    glViewport(0, 0, width, height);

}



void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    if (currentState != GameState::PLAYING) return;



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

    if (currentState == GameState::PLAYING)

        camera.ProcessMouseScroll(static_cast<float>(yoffset));

}



void PlayerTakeDamage(float damage) {

    if (playerHealth > 0) {

        playerHealth -= damage;

        if (playerHealth < 0) playerHealth = 0;

        // --- ATTIVA L'EFFETTO VISIVO E SONORO ---
        damageEffectTimer.Start();
        cameraShakeTimer.Start(); // <-- ATTIVA IL TREMORE
        if (playerHurtSound) SoundEngine->play2D(playerHurtSound, false);

        std::cout << "Player health: " << playerHealth << std::endl;

    }

}





void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (currentState != GameState::PLAYING) return;



    GameContext* context = static_cast<GameContext*>(glfwGetWindowUserPointer(window));

    if (!context) return;



    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

        context->sword->Attack();

        if (attackSound) SoundEngine->play2D(attackSound, false);



        const float SWORD_ATTACK_RANGE = 3.5f;



        // Controlla se il colpo raggiunge il Minotauro

        if (context->minotaur->health > 0 && glm::distance(context->camera->Position, context->minotaur->position) < SWORD_ATTACK_RANGE) {

            bool minotaurWasAlive = context->minotaur->health > 0;

            context->minotaur->TakeDamage(playerAttackDamage);



            if (context->minotaur->health > 0) {

                if (minotaurHitSound) SoundEngine->play2D(minotaurHitSound, false);

            }

            else if (minotaurWasAlive) {

                if (minotaurDeathSound) SoundEngine->play2D(minotaurDeathSound, false);

                victoryMessageTimer.Start();
                // Sblocca il filo di Arianna e avvia il timer del messaggio
                ariadneThreadUnlocked = true;
                unlockMessageTimer.Start();

            }

        }



        

        for (auto& room : *context->dungeonRooms) {

            // Controlla i nemici solo se la stanza è attiva

            if (room.state == DungeonRoom::RoomState::ACTIVE) {

                for (auto& enemy : room.enemies) {

                    // Controlla solo i nemici vivi e nel raggio d'azione

                    if (enemy.health > 0 && glm::distance(context->camera->Position, enemy.position) < SWORD_ATTACK_RANGE) {

                        enemy.TakeDamage(playerAttackDamage);

                        std::cout << "Colpito guardiano! Salute rimanente: " << enemy.health << std::endl;



                        // Puoi usare lo stesso suono di colpo o uno diverso

                        if (minotaurHitSound) SoundEngine->play2D(minotaurHitSound, false);



                        // Se il nemico muore, potresti far partire un suono di morte specifico

                        if (enemy.health <= 0) {

                            std::cout << "Guardiano sconfitto!" << std::endl;

                        }

                    }

                }

            }

        }

    }

}



void ResetGame(Camera& cam, Minotaur& minotaur) {

    std::cout << "--- GAME RESET ---" << std::endl;

    playerHealth = PLAYER_MAX_HEALTH;

    playerStamina = PLAYER_MAX_STAMINA;

    playerAttackDamage = 50.0f; 



    cam.Reset();

    minotaur.Reset();



    // Ricarica le casse per la nuova partita

    glm::vec3 minoStart;

    loadLevelData(minoStart);



    // Resetta ogni stanza dungeon (che a sua volta resetta casse e nemici)

    for (auto& room : dungeonRooms) {

        room.Reset();

    }



    SoundEngine->stopAllSounds();

    if (mainTheme) SoundEngine->play2D(mainTheme, true);



    firstMouse = true;

    hintPath.clear();

    hintTimer = 0.0f;

    victoryMessageTimer.Stop();

    powerUpMessageTimer.Stop();

    ariadneThreadUnlocked = false;
    unlockMessageTimer.Stop();

}



void FindPathForHint(glm::vec2 start, glm::vec2 target, std::vector<glm::vec3>& path) {

    path.clear();

    int width = MAZE_WIDTH;

    int height = MAZE_HEIGHT;



    if (start.x < 0 || start.x >= width || start.y < 0 || start.y >= height ||

        target.x < 0 || target.x >= width || target.y < 0 || target.y >= height ||

        initial_maze_map[static_cast<int>(target.y)][static_cast<int>(target.x)] == 1) {

        return;

    }



    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> open_set;

    std::map<int, glm::vec2> parent;

    std::map<int, int> g_cost;



    for (int y = 0; y < height; ++y) {

        for (int x = 0; x < width; ++x) {

            g_cost[y * width + x] = std::numeric_limits<int>::max();

        }

    }



    open_set.push({ 0, start });

    g_cost[static_cast<int>(start.y) * width + static_cast<int>(start.x)] = 0;



    int moves[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

    bool pathFound = false;



    while (!open_set.empty()) {

        AStarNode current_node = open_set.top();

        glm::vec2 current = current_node.position;

        open_set.pop();



        if (current.x == target.x && current.y == target.y) {

            pathFound = true;

            break;

        }



        for (auto& move : moves) {

            glm::vec2 next = { current.x + move[0], current.y + move[1] };

            int nextY = static_cast<int>(next.y);

            int nextX = static_cast<int>(next.x);



            if (nextX >= 0 && nextX < width && nextY >= 0 && nextY < height && initial_maze_map[nextY][nextX] != 1) {

                int move_cost = 1;

                for (auto& check_move : moves) {

                    int checkX = nextX + check_move[0];

                    int checkY = nextY + check_move[1];

                    if (checkX >= 0 && checkX < width && checkY >= 0 && checkY < height && initial_maze_map[checkY][checkX] == 1) {

                        move_cost = 15;

                        break;

                    }

                }

                int new_g_cost = g_cost[static_cast<int>(current.y) * width + static_cast<int>(current.x)] + move_cost;

                if (new_g_cost < g_cost[nextY * width + nextX]) {

                    g_cost[nextY * width + nextX] = new_g_cost;

                    int heuristic = abs(nextX - static_cast<int>(target.x)) + abs(nextY - static_cast<int>(target.y));

                    int f_cost = new_g_cost + heuristic;

                    open_set.push({ f_cost, next });

                    parent[nextY * width + nextX] = current;

                }

            }

        }

    }



    if (pathFound) {

        std::vector<glm::vec3> temp_path;

        glm::vec2 current = target;

        while (current.x != start.x || current.y != start.y) {

            temp_path.push_back(glm::vec3(current.x * CELL_SIZE + CELL_SIZE / 2.0f, 0.05f, current.y * CELL_SIZE + CELL_SIZE / 2.0f));

            int currentIndex = static_cast<int>(current.y) * width + static_cast<int>(current.x);

            if (parent.find(currentIndex) == parent.end()) break;

            current = parent[currentIndex];

        }

        temp_path.push_back(glm::vec3(start.x * CELL_SIZE + CELL_SIZE / 2.0f, 0.05f, start.y * CELL_SIZE + CELL_SIZE / 2.0f));

        std::reverse(temp_path.begin(), temp_path.end());

        path = temp_path;

    }

}



void processInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)

        glfwSetWindowShouldClose(window, true);



    GameContext* context = static_cast<GameContext*>(glfwGetWindowUserPointer(window));

    if (!context) return;

    Minotaur& minotaur = *context->minotaur;

    Camera& camera = *context->camera;

    //std::vector<Chest>& chests = *context->chests;

    std::vector<DungeonRoom>& rooms = *context->dungeonRooms;



    switch (currentState) {

    case GameState::MENU:

    case GameState::VICTORY:

    case GameState::GAME_OVER: {

        static float lastEnterPressTime = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {

            if (glfwGetTime() - lastEnterPressTime > 0.5f) {

                ResetGame(camera, minotaur);

                currentState = GameState::PLAYING;

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                lastEnterPressTime = static_cast<float>(glfwGetTime());

            }

        }

        break;

    }



    case GameState::PLAYING: {

        if (playerHealth <= 0) {

            currentState = GameState::GAME_OVER;

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            SoundEngine->stopAllSounds();

            break;

        }



        static bool e_key_pressed_debounce = false;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !e_key_pressed_debounce) {

            e_key_pressed_debounce = true;

            for (auto& room : rooms) {

                if (!room.chest.isCollected && glm::distance(camera.Position, room.chest.position) < 2.5f) {



                    if (room.chest.Open()) {

                        if (room.chest.powerUp == PowerUpType::HEALTH_BOOST) {

                            playerHealth = PLAYER_MAX_HEALTH;

                            lastPowerUpMessage = "Nettare degli Dei! Salute ripristinata!";

                        }

                        else if (room.chest.powerUp == PowerUpType::DAMAGE_BOOST) {

                            playerAttackDamage += 20.0f;

                            lastPowerUpMessage = "Furia di Ares! Danno aumentato a " + std::to_string(static_cast<int>(playerAttackDamage));





                        }

                        powerUpMessageTimer.Start();
                        break;
                    }

                    break;

                }

            }

        }

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {

            e_key_pressed_debounce = false;

        }





        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && ariadneThreadUnlocked) {

            if (!showHint) {

                showHint = true;

                hintTimer = HINT_DURATION;

                glm::vec2 startNode = { floor(camera.Position.x / CELL_SIZE), floor(camera.Position.z / CELL_SIZE) };

                glm::vec2 endNode = { -1, -1 };

                for (int y = 0; y < MAZE_HEIGHT; ++y) {

                    for (int x = 0; x < MAZE_WIDTH; ++x) {

                        if (initial_maze_map[y][x] == 3) {

                            endNode = glm::vec2(x, y);

                            break;

                        }

                    }

                    if (endNode.x != -1) break;

                }

                if (endNode.x != -1) {

                    FindPathForHint(startNode, endNode, hintPath);

                    if (!hintPath.empty()) {

                        glBindBuffer(GL_ARRAY_BUFFER, hintVBO);

                        glBufferSubData(GL_ARRAY_BUFFER, 0, hintPath.size() * sizeof(glm::vec3), &hintPath[0]);

                        glBindBuffer(GL_ARRAY_BUFFER, 0);

                    }

                }

            }

        }



        // --- Logica per mostrare le coordinate nel prompt con il tasto 'P' ---

        static bool p_key_pressed = false;

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !p_key_pressed) {

            p_key_pressed = true;

            // Stampa le coordinate direttamente sulla console

            std::cout << "Posizione Giocatore -> X: " << camera.Position.x << ", Z: " << camera.Position.z << std::endl;

        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {

            p_key_pressed = false;

        }



        // --- INIZIO LOGICA STAMINA E SCATTO ---

        bool isSprinting = false;

        // Controlla se il tasto SHIFT è premuto e se c'è stamina residua

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && playerStamina > 0) {

            isSprinting = true;

            playerStamina -= SPRINT_DRAIN_RATE * deltaTime; // Consuma stamina

            if (playerStamina < 0) playerStamina = 0;

        }

        else {

            // Altrimenti, rigenera la stamina se non è al massimo

            if (playerStamina < PLAYER_MAX_STAMINA) {

                playerStamina += STAMINA_REGEN_RATE * deltaTime;

                if (playerStamina > PLAYER_MAX_STAMINA) playerStamina = PLAYER_MAX_STAMINA;

            }

        }



        // Determina la velocità di movimento attuale

        const float NORMAL_SPEED = 3.5f;

        const float SPRINT_SPEED = 6.5f;

        float currentMoveSpeed = isSprinting ? SPRINT_SPEED : NORMAL_SPEED;

        // --- FINE LOGICA STAMINA ---





        glm::vec3 originalPosition = camera.Position;

        // float moveSpeed = 3.5f; // Rimossa la velocità fissa

        glm::vec3 desiredMovement(0.0f);



        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) desiredMovement += camera.Front;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) desiredMovement -= camera.Front;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) desiredMovement -= camera.Right;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) desiredMovement += camera.Right;



        if (glm::length(desiredMovement) > 0.0f) {

            desiredMovement.y = 0.0f;

            // Usa la nuova velocità dinamica calcolata dalla logica della stamina

            desiredMovement = glm::normalize(desiredMovement) * currentMoveSpeed * deltaTime;

        }



        // La tua logica di collisione e movimento rimane invariata

        if (glm::length(desiredMovement) > 0.0f) {

            glm::vec3 finalPosition = originalPosition;

            finalPosition.x += desiredMovement.x;

            if (checkWallCollision(finalPosition)) {

                finalPosition.x = originalPosition.x;

            }

            finalPosition.z += desiredMovement.z;

            if (checkWallCollision(finalPosition)) {

                finalPosition.z = originalPosition.z;

            }

            camera.Position = finalPosition;

        }



        if (checkMinotaurCollision(camera.Position, minotaur)) {

            glm::vec2 playerPos2D(camera.Position.x, camera.Position.z);

            glm::vec2 minotaurPos2D(minotaur.position.x, minotaur.position.z);

            float combinedRadius = CAMERA_COLLISION_RADIUS + minotaur.collisionRadius;

            float distance2D = glm::distance(playerPos2D, minotaurPos2D);



            if (distance2D < 0.0001f) {

                camera.Position.x += combinedRadius;

            }

            else {

                float penetrationDepth = combinedRadius - distance2D;

                glm::vec2 pushDirection2D = glm::normalize(playerPos2D - minotaurPos2D);

                glm::vec2 correction2D = pushDirection2D * (penetrationDepth + 0.01f);

                camera.Position.x += correction2D.x;

                camera.Position.z += correction2D.y;

            }

            if (checkWallCollision(camera.Position)) {

                camera.Position = originalPosition;

            }

        }



        if (originalPosition.z < EXIT_Z_THRESHOLD && camera.Position.z >= EXIT_Z_THRESHOLD) {

            if (camera.Position.x > EXIT_X_MIN && camera.Position.x < EXIT_X_MAX) {

                currentState = GameState::VICTORY;

                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                SoundEngine->stopAllSounds();

            }

        }



        camera.Position.y = CAMERA_HEIGHT;

        break;

    }

    }

}





void loadLevelData(glm::vec3& minotaurSpawnPos) {

    // Pulisci le casse dalla partita precedente

    if (chestModel_ptr) {

        chests.clear();

    }



    minotaurSpawnPos = glm::vec3(-1.0f);

    for (int y = 0; y < MAZE_HEIGHT; ++y) {

        for (int x = 0; x < MAZE_WIDTH; ++x) {

            maze[y][x].wall = (initial_maze_map[y][x] == 1);



            float worldX = x * CELL_SIZE + CELL_SIZE / 2.0f;

            float worldZ = y * CELL_SIZE + CELL_SIZE / 2.0f;



            if (initial_maze_map[y][x] == 2) {

                minotaurSpawnPos = glm::vec3(worldX, 0.0f, worldZ);

            }

            else if (initial_maze_map[y][x] == 4 && chestModel_ptr && enemyModel_ptr) {

                // Se troviamo una cassa (4), la creiamo e la aggiungiamo al vettore

                //chests.emplace_back(*chestModel_ptr, glm::vec3(worldX, 0.5f, worldZ));

                // Definisce i confini della stanza (es. un'area 5x5 intorno al marcatore)

                glm::vec4 bounds = {

                    (x - 2) * CELL_SIZE, (x + 3) * CELL_SIZE,

                    (y - 2) * CELL_SIZE, (y + 3) * CELL_SIZE

                };



                // Crea la cassa al centro della stanza (altezza sistemata)

                Chest chest(*chestModel_ptr, glm::vec3(worldX, 0.5f, worldZ));



                // Crea i nemici guardiani ai lati della cassa

                std::vector<Enemy> enemies;

                enemies.emplace_back(*enemyModel_ptr, glm::vec3(worldX - 1.5f, 0.0f, worldZ - 1.5f), bounds);

                enemies.emplace_back(*enemyModel_ptr, glm::vec3(worldX + 1.5f, 0.0f, worldZ + 1.5f), bounds);



                dungeonRooms.emplace_back(chest, std::move(enemies), bounds);

            }

        }

    }

    if (minotaurSpawnPos.x < 0.0f) {

        minotaurSpawnPos = glm::vec3(10.0f, 0.0f, 10.0f);

    }

}







bool checkWallCollision(glm::vec3 checkPos) {

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

                    return true;

                }

            }

        }

    }

    return false;

}



bool checkMinotaurCollision(glm::vec3 checkPos, const Minotaur& minotaur) {

    if (minotaur.currentState != Minotaur::State::FINISHED) {

        float distance = glm::distance(glm::vec2(checkPos.x, checkPos.z), glm::vec2(minotaur.position.x, minotaur.position.z));

        if (distance < minotaur.collisionRadius + CAMERA_COLLISION_RADIUS) {

            return true;

        }

    }

    return false;

}



//nuovo

bool checkCollision(glm::vec3 checkPos, const Minotaur& minotaur) {

    // 1. Collisione con i muri

    if (checkWallCollision(checkPos)) return true;



    // 2. Collisione con il Minotauro

    if (checkMinotaurCollision(checkPos, minotaur)) return true;



    // 3. Collisione con i nemici delle stanze

    for (const auto& room : dungeonRooms) {

        if (room.state == DungeonRoom::RoomState::ACTIVE) {

            for (const auto& enemy : room.enemies) {

                if (enemy.health > 0) {

                    if (glm::distance(glm::vec2(checkPos.x, checkPos.z), glm::vec2(enemy.position.x, enemy.position.z)) < enemy.collisionRadius + CAMERA_COLLISION_RADIUS) {

                        return true;

                    }

                }

            }

        }

    }



    return false; // Nessuna collisione

}



// Funzione per caricare una texture da un file

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

            std::cerr << "Unsupported image format: " << path << std::endl;

            stbi_image_free(data);

            return 0;

        }

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

        std::cerr << "Texture failed to load: " << path << std::endl;

        return 0;

    }

    return textureID;

}



// Funzione per caricare texture per i modelli 3D

inline unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma) {

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



// Funzione per configurare la geometria del menu

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



void setupMazeGeometryVAOs() {

    float cubeVertices[] = {

        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,

         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,

         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,

         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,

         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,    0.0f, 0.0f, -1.0f

    };

    glGenBuffers(1, &VBO_cube_lit);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);



    glGenVertexArrays(1, &VAO_walls);

    glBindVertexArray(VAO_walls);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    glEnableVertexAttribArray(4);



    float mazeW = (float)MAZE_WIDTH * CELL_SIZE;

    float mazeD = (float)MAZE_HEIGHT * CELL_SIZE;

    float textureRepeatX = mazeW / CELL_SIZE;

    float textureRepeatZ = mazeD / CELL_SIZE;



    float floorVertices[] = {

        mazeW, 0.0f, mazeD,    0.0f, 1.0f, 0.0f,  textureRepeatX, textureRepeatZ, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        mazeW, 0.0f, 0.0f,     0.0f, 1.0f, 0.0f,  textureRepeatX, 0.0f,           1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        0.0f,  0.0f, 0.0f,     0.0f, 1.0f, 0.0f,  0.0f,           0.0f,           1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        0.0f,  0.0f, mazeD,    0.0f, 1.0f, 0.0f,  0.0f,           textureRepeatZ, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    glEnableVertexAttribArray(4);



    float ceilingVertices[] = {

        mazeW, WALL_HEIGHT, mazeD,   0.0f, -1.0f, 0.0f,  textureRepeatX, textureRepeatZ, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        0.0f,  WALL_HEIGHT, mazeD,   0.0f, -1.0f, 0.0f,  0.0f,           textureRepeatZ, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        0.0f,  WALL_HEIGHT, 0.0f,    0.0f, -1.0f, 0.0f,  0.0f,           0.0f,           1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        mazeW, WALL_HEIGHT, 0.0f,    0.0f, -1.0f, 0.0f,  textureRepeatX, 0.0f,           1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f

    };

    unsigned int ceilingIndices[] = { 0, 1, 3, 1, 2, 3 };

    unsigned int VBO_ceiling, EBO_ceiling;

    glGenVertexArrays(1, &VAO_ceiling);

    glGenBuffers(1, &VBO_ceiling);

    glGenBuffers(1, &EBO_ceiling);

    glBindVertexArray(VAO_ceiling);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_ceiling);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ceilingVertices), ceilingVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ceiling);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ceilingIndices), ceilingIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    glEnableVertexAttribArray(4);



    glGenVertexArrays(1, &VAO_lamp);

    glBindVertexArray(VAO_lamp);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube_lit);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);



    glBindVertexArray(0);

}