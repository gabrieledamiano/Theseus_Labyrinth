#ifndef MINOTAUR_H
#define MINOTAUR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include <queue> 
#include <string>

#include "shader_m.h"
#include "model.h"
#include "animator.h"
#include "Cell.h"
#include "config.h"

// Dichiarazione della funzione globale definita in main.cpp
void PlayerTakeDamage(float damage);

class Minotaur {
public:
    // Stati possibili del Minotauro
    enum class State {
        IDLE,
        WALKING,
        ATTACKING,
        GET_HIT,
        DEATH
    };

    Model& minotaurModel;
    Animator animator;
    const std::vector<std::vector<Cell>>& mazeLayout;

    // Attributi
    glm::vec3 position;
    float orientation;
    float speed;
    float health;
    float collisionRadius;

    // Stato e timer
    State currentState;
    float stateTimer;
    bool didDealDamage;

    // Costanti per la configurazione dell'IA
    const float ATTACK_RANGE = 2.5f;
    const float NOTICE_RANGE = 20.0f;
    const float ATTACK_ANIM_DURATION = 1.5f;
    const float PATH_REEVALUATION_COOLDOWN = 1.0f;

private:
    std::vector<glm::vec2> m_path;
    float pathReevaluationTimer;

public:
    Minotaur(Model& model, glm::vec3 startPos, const std::vector<std::vector<Cell>>& maze)
        : minotaurModel(model),
        animator(model.m_Animations["idle"]),
        position(startPos),
        mazeLayout(maze),
        orientation(0.0f),
        speed(2.2f),
        health(100.0f),
        collisionRadius(0.8f),
        currentState(State::IDLE),
        stateTimer(0.0f),
        didDealDamage(false),
        pathReevaluationTimer(0.0f)
    {
        position.y = 0.0f;
    }

    void SetState(State newState) {
        if (currentState == newState || currentState == State::DEATH) return;

        currentState = newState;
        stateTimer = 0.0f; // Resetta il timer ad ogni cambio di stato

        // Cambia l'animazione in base al nuovo stato
        switch (newState) {
        case State::IDLE:
            if (minotaurModel.m_Animations.count("idle")) animator.PlayAnimation(minotaurModel.m_Animations["idle"]);
            m_path.clear();
            break;
        case State::WALKING:
            if (minotaurModel.m_Animations.count("walk")) animator.PlayAnimation(minotaurModel.m_Animations["walk"]);
            break;
        case State::ATTACKING:
            if (minotaurModel.m_Animations.count("attack")) animator.PlayAnimation(minotaurModel.m_Animations["attack"]);
            didDealDamage = false;
            break;
        case State::GET_HIT:
            if (minotaurModel.m_Animations.count("get_hit")) animator.PlayAnimation(minotaurModel.m_Animations["get_hit"]);
            break;
        case State::DEATH:
            if (minotaurModel.m_Animations.count("death")) animator.PlayAnimation(minotaurModel.m_Animations["death"]);
            break;
        }
    }

    void TakeDamage(float damage) {
        if (currentState == State::DEATH) return;
        health -= damage;
        if (health <= 0) {
            health = 0;
            SetState(State::DEATH);
        }
        else {
            SetState(State::GET_HIT);
        }
    }

    void Update(float deltaTime, glm::vec3 playerPosition) {
        animator.UpdateAnimation(deltaTime);
        pathReevaluationTimer -= deltaTime;

        // Se è morto, non fa assolutamente nient'altro.
        if (currentState == State::DEATH) return;

        // Se è in uno stato temporaneo (colpito o sta attaccando), gestisce solo il timer.
        // Al termine dello stato, passa a IDLE per forzare una rivalutazione completa.
        if (currentState == State::GET_HIT) {
            stateTimer += deltaTime;
            if (stateTimer >= 0.5f) { // Durata dello stordimento
                SetState(State::IDLE);
            }
            return; // Esce per questo frame, non prende altre decisioni.
        }
        if (currentState == State::ATTACKING) {
            stateTimer += deltaTime;
            orientation = glm::degrees(atan2(playerPosition.x - position.x, playerPosition.z - position.z));

            if (stateTimer > ATTACK_ANIM_DURATION / 2.0f && !didDealDamage) {
                if (glm::distance(playerPosition, position) < ATTACK_RANGE + 0.5f) {
                    PlayerTakeDamage(15.0f);
                }
                didDealDamage = true;
            }
            if (stateTimer >= ATTACK_ANIM_DURATION) {
                SetState(State::IDLE);
            }
            return; // Esce per questo frame, non prende altre decisioni.
        }

        // Se non è bloccato, può prendere decisioni basate sulla distanza.
        float distanceToPlayer = glm::distance(playerPosition, position);
        if (distanceToPlayer < ATTACK_RANGE) {
            SetState(State::ATTACKING);
        }
        else if (distanceToPlayer < NOTICE_RANGE) {
            SetState(State::WALKING);
        }
        else {
            SetState(State::IDLE);
        }

        // Esegue l'azione solo per gli stati principali (WALKING, IDLE).
        if (currentState == State::WALKING) {
            if (pathReevaluationTimer <= 0.0f || m_path.empty()) {
                glm::vec2 startNode = { floor(position.x / CELL_SIZE), floor(position.z / CELL_SIZE) };
                glm::vec2 endNode = { floor(playerPosition.x / CELL_SIZE), floor(playerPosition.z / CELL_SIZE) };
                FindPath(startNode, endNode);
                pathReevaluationTimer = PATH_REEVALUATION_COOLDOWN;
            }
            MoveAlongPath(deltaTime);
        }
        else if (currentState == State::IDLE) {
            if (distanceToPlayer < NOTICE_RANGE) {
                orientation = glm::degrees(atan2(playerPosition.x - position.x, playerPosition.z - position.z));
            }
        }
    }

    void Draw(Shader& shader) {
        shader.use();
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i) {
            shader.setMat4("finalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(orientation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.02f));

        shader.setMat4("model", model);
        minotaurModel.Draw(shader);
    }

private:
    void MoveAlongPath(float deltaTime) {
        if (m_path.empty()) {
            SetState(State::IDLE);
            return;
        }

        glm::vec2 targetGridPos = m_path.back();
        glm::vec3 targetWorldPos = glm::vec3(targetGridPos.x * CELL_SIZE + CELL_SIZE / 2.0f, position.y, targetGridPos.y * CELL_SIZE + CELL_SIZE / 2.0f);

        glm::vec3 direction = glm::normalize(targetWorldPos - position);

        position += direction * speed * deltaTime;
        orientation = glm::degrees(atan2(direction.x, direction.z));

        if (glm::distance(glm::vec2(position.x, position.z), glm::vec2(targetWorldPos.x, targetWorldPos.z)) < 0.2f) {
            m_path.pop_back();
        }
    }

    void FindPath(glm::vec2 start, glm::vec2 target) {
        m_path.clear();
        int width = mazeLayout[0].size();
        int height = mazeLayout.size();

        if (start.x < 0 || start.x >= width || start.y < 0 || start.y >= height ||
            target.x < 0 || target.x >= width || target.y < 0 || target.y >= height ||
            mazeLayout[target.y][target.x].wall) {
            return;
        }

        queue<glm::vec2> q;
        q.push(start);

        map<int, glm::vec2> parent;
        vector<bool> visited(width * height, false);
        visited[start.y * width + start.x] = true;

        int moves[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
        bool pathFound = false;

        while (!q.empty()) {
            glm::vec2 current = q.front();
            q.pop();

            if (current.x == target.x && current.y == target.y) {
                pathFound = true;
                break;
            }

            for (auto& move : moves) {
                glm::vec2 next = { current.x + move[0], current.y + move[1] };
                if (next.x >= 0 && next.x < width && next.y >= 0 && next.y < height &&
                    !mazeLayout[next.y][next.x].wall && !visited[next.y * width + next.x]) {
                    visited[next.y * width + next.x] = true;
                    parent[next.y * width + next.x] = current;
                    q.push(next);
                }
            }
        }

        if (pathFound) {
            glm::vec2 current = target;
            while (current.x != start.x || current.y != start.y) {
                m_path.push_back(current);
                current = parent[current.y * width + current.x];
            }
        }
    }
};

#endif