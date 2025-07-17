#ifndef MINOTAUR_H
#define MINOTAUR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <algorithm>
#include <limits> 
#include <random> 

#include "shader_m.h"
#include "model.h"
#include "animator.h"
#include "Cell.h"
#include "config.h"

// Forward declaration della funzione globale definita in main.cpp
void PlayerTakeDamage(float damage);

class Minotaur {
private:
    // Struct per il risultato della collisione, contiene la normale del muro
    struct CollisionResult {
        bool hasCollided = false;
        glm::vec3 collisionNormal = glm::vec3(0.0f);
    };

    // Struct personalizzata per l'algoritmo A*, che sa come essere confrontata
    struct AStarNode {
        int total_cost;
        glm::vec2 position;

        bool operator>(const AStarNode& other) const {
            return total_cost > other.total_cost;
        }
    };

public:
    // Macchina a stati completa con i nuovi stati per il comportamento avanzato
    enum class State { IDLE, WALKING, ATTACKING, GET_HIT, DEATH, FINISHED, PATROLLING, RETURNING };

    // Riferimenti e dati principali
    Model& minotaurModel;
    Animator animator;
    const std::vector<std::vector<Cell>>& mazeLayout;

    // Proprietà fisiche e di stato
    glm::vec3 position;
    glm::vec3 spawnPosition; // Memorizza il punto di spawn
    float orientation;
    float speed;
    float health;
    float collisionRadius;

    // Gestione della macchina a stati
    State currentState;
    float stateTimer;
    bool didDealDamage;

    // Costanti di comportamento
    const float ATTACK_RANGE = 2.8f;
    const float NOTICE_RANGE = 20.0f;
    const float ATTACK_ANIM_DURATION = 1.5f;
    const float GET_HIT_DURATION = 0.5f;
    const float PATH_REEVALUATION_COOLDOWN = 1.0f;
    const float DEATH_DELAY = 2.0f;
    const float IDLE_TIMEOUT_BEFORE_ACTION = 5.0f; // Dopo 5 sec di inattività, fa qualcosa

private:
    std::vector<glm::vec3> m_path_world;
    int currentPathIndex;
    float pathReevaluationTimer;
    float idleTimer; // Timer per l'inattività

public:
    // Costruttore
    Minotaur(Model& model, glm::vec3 startPos, const std::vector<std::vector<Cell>>& maze)
        : minotaurModel(model),
        animator(model.m_Animations["idle"]),
        position(startPos),
        spawnPosition(startPos), // Memorizza la posizione iniziale
        mazeLayout(maze),
        orientation(0.0f),
        speed(2.9f),
        health(3000.0f),
        collisionRadius(0.45f),
        currentState(State::IDLE),
        stateTimer(0.0f),
        didDealDamage(false),
        pathReevaluationTimer(0.0f),
        currentPathIndex(0),
        idleTimer(0.0f) // Inizializza il timer di inattività
    {
        position.y = 0.0f;
    }

    // Imposta un nuovo stato per l'IA e riproduce l'animazione corrispondente
    void SetState(State newState) {
        if (currentState == newState || currentState == State::FINISHED) return;

        currentState = newState;
        stateTimer = 0.0f;

        if (newState == State::IDLE || newState == State::PATROLLING || newState == State::RETURNING) {
            m_path_world.clear();
            currentPathIndex = 0;
        }
        if (newState == State::IDLE) {
            idleTimer = 0.0f;
        }

        switch (newState) {
        case State::IDLE:      if (minotaurModel.m_Animations.count("idle")) animator.PlayAnimation(minotaurModel.m_Animations["idle"]); break;
        case State::WALKING:
        case State::PATROLLING:
        case State::RETURNING:
            if (minotaurModel.m_Animations.count("walk")) animator.PlayAnimation(minotaurModel.m_Animations["walk"]); break;
        case State::ATTACKING:
            if (minotaurModel.m_Animations.count("attack")) animator.PlayAnimation(minotaurModel.m_Animations["attack"]);
            didDealDamage = false;
            break;
        case State::GET_HIT:   if (minotaurModel.m_Animations.count("get_hit")) animator.PlayAnimation(minotaurModel.m_Animations["get_hit"]); break;
        case State::DEATH:     if (minotaurModel.m_Animations.count("death")) animator.PlayAnimation(minotaurModel.m_Animations["death"]); break;
        case State::FINISHED:  break;
        }
    }

    // Applica danno al Minotauro
    void TakeDamage(float damage) {
        if (currentState == State::DEATH || currentState == State::FINISHED) return;

        health -= damage;
        if (health <= 0) {
            health = 0;
            SetState(State::DEATH);
        }
        else {
            SetState(State::GET_HIT);
        }
    }

    
    void Reset() {
        // Ripristina la salute al valore iniziale
        health = 3000.0f; 

        // Riportalo alla sua posizione di spawn
        position = spawnPosition;

        // Imposta lo stato direttamente su IDLE, bypassando i controlli di SetState.
        // Questa è la correzione chiave del bug.
        currentState = State::IDLE;

        // Resetta manualmente anche le altre variabili di stato correlate
        idleTimer = 0.0f;
        stateTimer = 0.0f;
        m_path_world.clear();
        currentPathIndex = 0;

        // Fa ripartire anche l'animazione di idle per essere sicuri che sia visivamente corretto
        if (minotaurModel.m_Animations.count("idle")) {
            animator.PlayAnimation(minotaurModel.m_Animations["idle"]);
        }
    }

    // Funzione di aggiornamento principale
    void Update(float deltaTime, glm::vec3 playerPosition) {
        animator.UpdateAnimation(deltaTime);
        pathReevaluationTimer -= deltaTime;

        // --- Stati bloccanti ad alta priorità (invariati) ---
        if (currentState == State::FINISHED || currentState == State::DEATH) {
            if (currentState == State::DEATH) {
                stateTimer += deltaTime;
                if (stateTimer >= DEATH_DELAY) SetState(State::FINISHED);
            }
            return;
        }
        if (currentState == State::GET_HIT) {
            stateTimer += deltaTime;
            if (stateTimer >= GET_HIT_DURATION) SetState(State::IDLE);
            return;
        }

        // --- Logica di decisione e di stato CORRETTA ---

        float distanceToPlayer = glm::distance(playerPosition, position);

        // Se stiamo attaccando, completiamo l'attacco prima di decidere altro
        if (currentState == State::ATTACKING) {
            stateTimer += deltaTime;
            orientation = glm::degrees(atan2(playerPosition.x - position.x, playerPosition.z - position.z));
            if (stateTimer > ATTACK_ANIM_DURATION / 2.0f && !didDealDamage) {
                if (glm::distance(playerPosition, position) < ATTACK_RANGE + 0.5f) PlayerTakeDamage(20.0f);
                didDealDamage = true;
            }
            // Se l'animazione di attacco è finita, torna IDLE per rivalutare.
            // NON passa direttamente a WALKING, ma aspetta il prossimo frame per decidere.
            if (stateTimer >= ATTACK_ANIM_DURATION) SetState(State::IDLE);
            return;
        }

        // --- Albero decisionale principale (con la priorità corretta) ---
        if (distanceToPlayer < ATTACK_RANGE) {
            // 1. PRIORITÀ MASSIMA: Se il giocatore è in raggio d'attacco, ATTACCA.
            SetState(State::ATTACKING);
        }
        else if (distanceToPlayer < NOTICE_RANGE) {
            // 2. ALTRIMENTI, se è in raggio di avviso, INSEGUI.
            SetState(State::WALKING);
        }
        else {
            // 3. ALTRIMENTI, se non vede più il giocatore, passa a IDLE.
            // Passa a IDLE solo se non lo era già, per evitare di resettare il timer.
            if (currentState == State::WALKING || currentState == State::PATROLLING || currentState == State::RETURNING) {
                SetState(State::IDLE);
            }
        }

        // Esegui l'azione corrispondente allo stato attuale (che non sia bloccante)
        switch (currentState) {
        case State::WALKING: {
            if (pathReevaluationTimer <= 0.0f || m_path_world.empty()) {
                glm::vec2 startNode = { floor(position.x / CELL_SIZE), floor(position.z / CELL_SIZE) };
                glm::vec2 endNode = { floor(playerPosition.x / CELL_SIZE), floor(playerPosition.z / CELL_SIZE) };
                FindPath(startNode, endNode);
                pathReevaluationTimer = PATH_REEVALUATION_COOLDOWN;
            }
            MoveAlongPath(deltaTime);
            break;
        }

        case State::IDLE: {
            idleTimer += deltaTime;
            if (idleTimer > IDLE_TIMEOUT_BEFORE_ACTION) {
                //SetState(State::RETURNING); // Default: torna allo spawn
                SetState(State::PATROLLING); // Alternativa: pattuglia
            }
            break;
        }

        case State::RETURNING: {
            if (m_path_world.empty() || currentPathIndex >= m_path_world.size()) {
                glm::vec2 startNode = { floor(position.x / CELL_SIZE), floor(position.z / CELL_SIZE) };
                glm::vec2 endNode = { floor(spawnPosition.x / CELL_SIZE), floor(spawnPosition.z / CELL_SIZE) };
                FindPath(startNode, endNode);
            }
            MoveAlongPath(deltaTime);
            if (glm::distance(position, spawnPosition) < CELL_SIZE) {
                SetState(State::IDLE);
            }
            break;
        }

        case State::PATROLLING: {
            if (m_path_world.empty() || currentPathIndex >= m_path_world.size()) {
                glm::vec2 startNode = { floor(position.x / CELL_SIZE), floor(position.z / CELL_SIZE) };
                FindPath(startNode, GetRandomPatrolPoint());
            }
            MoveAlongPath(deltaTime);
            break;
        }
        default:
            break;
        }
    }

    // Disegna il modello
    void Draw(Shader& shader) {
        shader.use();
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i) {
            shader.setMat4("finalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 visualPosition = position + glm::vec3(0.0f, 1.2f, 0.0f);
        model = glm::translate(model, visualPosition);
        model = glm::rotate(model, glm::radians(orientation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.3f));
        shader.setMat4("model", model);
        minotaurModel.Draw(shader);
    }

private:
    // Trova un punto casuale non-muro sulla mappa per il pattugliamento
    glm::vec2 GetRandomPatrolPoint() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> distribX(0, mazeLayout[0].size() - 1);
        std::uniform_int_distribution<> distribZ(0, mazeLayout.size() - 1);

        glm::vec2 randomPoint;
        do {
            randomPoint.x = distribX(gen);
            randomPoint.y = distribZ(gen);
        } while (mazeLayout[static_cast<int>(randomPoint.y)][static_cast<int>(randomPoint.x)].wall);

        return randomPoint;
    }

    // Muove il Minotauro con logica di scivolamento sui muri
    void MoveAlongPath(float deltaTime) {
        if (currentPathIndex >= m_path_world.size()) {
            SetState(State::IDLE); // Se il percorso finisce, torna IDLE
            return;
        }

        glm::vec3 primary_target_pos = m_path_world[currentPathIndex];
        primary_target_pos.y = position.y;

        if (glm::distance(position, primary_target_pos) < 0.1f) {
            currentPathIndex++;
            if (currentPathIndex >= m_path_world.size()) {
                SetState(State::IDLE);
                return;
            }
            primary_target_pos = m_path_world[currentPathIndex];
            primary_target_pos.y = position.y;
        }

        glm::vec3 primary_direction = glm::normalize(primary_target_pos - position);
        glm::vec3 final_direction = primary_direction;

        if (currentPathIndex + 1 < m_path_world.size()) {
            glm::vec3 look_ahead_pos = m_path_world[currentPathIndex + 1];
            look_ahead_pos.y = position.y;
            glm::vec3 look_ahead_direction = glm::normalize(look_ahead_pos - position);
            final_direction = glm::normalize((primary_direction * 0.7f) + (look_ahead_direction * 0.3f));
        }

        if (glm::length(final_direction) > 0.0f) {
            glm::vec3 velocity = final_direction * speed * deltaTime;
            glm::vec3 nextPos = position + velocity;
            CollisionResult collision = CheckMazeCollision(nextPos);

            if (collision.hasCollided) {
                float projection = glm::dot(velocity, collision.collisionNormal);
                glm::vec3 slideVector = velocity - (collision.collisionNormal * projection);
                nextPos = position + slideVector;

                collision = CheckMazeCollision(nextPos);
                if (!collision.hasCollided) {
                    position = nextPos;
                }
            }
            else {
                position = nextPos;
            }
            orientation = glm::degrees(atan2(final_direction.x, final_direction.z));
        }

        const float WAYPOINT_PROXIMITY = CELL_SIZE * 0.7f;
        if (glm::distance(position, primary_target_pos) < WAYPOINT_PROXIMITY) {
            currentPathIndex++;
        }
    }

    // Calcola il percorso con A* e costo dei muri
    void FindPath(glm::vec2 start, glm::vec2 target) {
        m_path_world.clear();
        currentPathIndex = 0;
        int width = mazeLayout[0].size();
        int height = mazeLayout.size();

        if (start.x < 0 || start.x >= width || start.y < 0 || start.y >= height ||
            target.x < 0 || target.x >= width || target.y < 0 || target.y >= height ||
            mazeLayout[static_cast<int>(target.y)][static_cast<int>(target.x)].wall) {
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

                if (nextX >= 0 && nextX < width && nextY >= 0 && nextY < height && !mazeLayout[nextY][nextX].wall) {
                    int move_cost = 1;
                    for (auto& check_move : moves) {
                        int checkX = nextX + check_move[0];
                        int checkY = nextY + check_move[1];
                        if (checkX >= 0 && checkX < width && checkY >= 0 && checkY < height && mazeLayout[checkY][checkX].wall) {
                            move_cost = 15; // Aumentato costo per rendere l'evitamento più marcato
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
            std::vector<glm::vec2> gridPath;
            glm::vec2 current = target;
            while (current.x != start.x || current.y != start.y) {
                gridPath.push_back(current);
                int currentIndex = static_cast<int>(current.y) * width + static_cast<int>(current.x);
                if (parent.find(currentIndex) == parent.end()) break;
                current = parent[currentIndex];
            }
            std::reverse(gridPath.begin(), gridPath.end());
            for (const auto& node : gridPath) {
                m_path_world.push_back(glm::vec3(node.x * CELL_SIZE + CELL_SIZE / 2.0f, position.y, node.y * CELL_SIZE + CELL_SIZE / 2.0f));
            }
        }
    }

    // Controlla la collisione e restituisce i dettagli dell'impatto
    CollisionResult CheckMazeCollision(glm::vec3 checkPos) {
        CollisionResult result;
        int gridX = static_cast<int>(floor(checkPos.x / CELL_SIZE));
        int gridZ = static_cast<int>(floor(checkPos.z / CELL_SIZE));

        for (int z = gridZ - 1; z <= gridZ + 1; ++z) {
            for (int x = gridX - 1; x <= gridX + 1; ++x) {
                if (x >= 0 && x < mazeLayout[0].size() && z >= 0 && z < mazeLayout.size() && mazeLayout[z][x].wall) {
                    float wallX = (x * CELL_SIZE) + (CELL_SIZE / 2.0f);
                    float wallZ = (z * CELL_SIZE) + (CELL_SIZE / 2.0f);
                    float closestX = std::max(wallX - CELL_SIZE / 2.0f, std::min(checkPos.x, wallX + CELL_SIZE / 2.0f));
                    float closestZ = std::max(wallZ - CELL_SIZE / 2.0f, std::min(checkPos.z, wallZ + CELL_SIZE / 2.0f));
                    float distance = glm::distance(glm::vec2(closestX, closestZ), glm::vec2(checkPos.x, checkPos.z));

                    if (distance < collisionRadius) {
                        result.hasCollided = true;
                        glm::vec2 diff = glm::vec2(checkPos.x, checkPos.z) - glm::vec2(wallX, wallZ);
                        if (abs(diff.x) > abs(diff.y)) {
                            result.collisionNormal = glm::vec3(glm::sign(diff.x), 0.0f, 0.0f);
                        }
                        else {
                            result.collisionNormal = glm::vec3(0.0f, 0.0f, glm::sign(diff.y));
                        }
                        return result;
                    }
                }
            }
        }
        return result;
    }
};

#endif