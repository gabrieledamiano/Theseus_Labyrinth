#ifndef MINOTAUR_H
#define MINOTAUR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include "shader_m.h"
#include "model.h"
#include "animator.h"
#include "Cell.h"

void PlayerTakeDamage(float damage);

class Minotaur {
public:
    enum class State { IDLE, WALKING, ATTACKING, GET_HIT, DEATH };

    Model& minotaurModel;
    Animator animator;
    const std::vector<std::vector<Cell>>& mazeLayout;

    glm::vec3 position;
    float orientation;
    float speed;
    float health;
    float collisionRadius = 0.8f;

    State currentState;
    float stateTimer;
    bool didDealDamage;

    const float ATTACK_RANGE = 2.5f;
    const float NOTICE_RANGE = 15.0f;
    const float ATTACK_ANIM_DURATION = 1.5f;

    Minotaur(Model& model, glm::vec3 startPos, const std::vector<std::vector<Cell>>& maze)
        : minotaurModel(model),
        animator(model.m_Animations["idle"]),
        position(startPos),
        mazeLayout(maze),
        orientation(0.0f),
        speed(1.8f),
        health(100.0f),
        currentState(State::IDLE),
        stateTimer(0.0f),
        didDealDamage(false)
    {
        position.y = 0.0f;
    }

    void SetState(State newState) {
        if (currentState == newState || currentState == State::DEATH) return;
        currentState = newState;
        stateTimer = 0.0f;

        switch (newState) {
        case State::IDLE: animator.PlayAnimation(minotaurModel.m_Animations["idle"]); break;
        case State::WALKING: animator.PlayAnimation(minotaurModel.m_Animations["walk"]); break;
        case State::ATTACKING:
            animator.PlayAnimation(minotaurModel.m_Animations["attack"]);
            didDealDamage = false;
            break;
        case State::GET_HIT: animator.PlayAnimation(minotaurModel.m_Animations["get_hit"]); break;
        case State::DEATH: animator.PlayAnimation(minotaurModel.m_Animations["death"]); break;
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

        if (currentState == State::DEATH || currentState == State::GET_HIT) {
            stateTimer += deltaTime;
            if (currentState == State::GET_HIT && stateTimer > 0.5f) {
                SetState(State::WALKING);
            }
            return;
        }

        float distanceToPlayer = glm::distance(playerPosition, position);

        if (distanceToPlayer < ATTACK_RANGE && currentState != State::ATTACKING) {
            SetState(State::ATTACKING);
        }
        else if (distanceToPlayer >= ATTACK_RANGE && distanceToPlayer < NOTICE_RANGE && currentState != State::WALKING) {
            SetState(State::WALKING);
        }
        else if (distanceToPlayer >= NOTICE_RANGE && currentState != State::IDLE) {
            SetState(State::IDLE);
        }

        switch (currentState) {
        case State::WALKING:
        {
            glm::vec3 direction = glm::normalize(playerPosition - position);
            direction.y = 0;

            glm::vec3 nextPos = position + direction * speed * deltaTime;

            // --- CORREZIONE QUI ---
            // Ora chiama la sua funzione di collisione interna, che accetta 1 argomento.
            if (!CheckMazeCollision(nextPos)) {
                position = nextPos;
            }

            orientation = glm::degrees(atan2(direction.x, direction.z));
            break;
        }
        case State::ATTACKING:
            stateTimer += deltaTime;
            orientation = glm::degrees(atan2(playerPosition.x - position.x, playerPosition.z - position.z));
            if (stateTimer > ATTACK_ANIM_DURATION / 2.0f && !didDealDamage) {
                if (distanceToPlayer < ATTACK_RANGE + 0.5f) {
                    PlayerTakeDamage(15.0f);
                }
                didDealDamage = true;
            }
            if (stateTimer > ATTACK_ANIM_DURATION) {
                SetState(State::WALKING);
            }
            break;
        case State::IDLE:
            break;
        }
    }

    // Funzione di collisione interna del Minotauro: controlla solo i muri
    bool CheckMazeCollision(glm::vec3 checkPos) {
        int gridX = static_cast<int>(floor(checkPos.x / 1.5f));
        int gridZ = static_cast<int>(floor(checkPos.z / 1.5f));
        float mazeWidth = mazeLayout[0].size();
        float mazeHeight = mazeLayout.size();

        for (int z = gridZ - 1; z <= gridZ + 1; ++z) {
            for (int x = gridX - 1; x <= gridX + 1; ++x) {
                if (x >= 0 && x < mazeWidth && z >= 0 && z < mazeHeight && mazeLayout[z][x].wall) {
                    float wallX = (x * 1.5f) + (1.5f / 2.0f);
                    float wallZ = (z * 1.5f) + (1.5f / 2.0f);
                    float closestX = std::max(wallX - 1.5f / 2.0f, std::min(checkPos.x, wallX + 1.5f / 2.0f));
                    float closestZ = std::max(wallZ - 1.5f / 2.0f, std::min(checkPos.z, wallZ + 1.5f / 2.0f));
                    if (glm::distance(glm::vec2(closestX, closestZ), glm::vec2(checkPos.x, checkPos.z)) < collisionRadius) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    void Draw(Shader& shader) {
        shader.use();
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            shader.setMat4("finalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.02f));
        model = glm::rotate(model, glm::radians(orientation), glm::vec3(3.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", model);
        minotaurModel.Draw(shader);
    }
};

#endif