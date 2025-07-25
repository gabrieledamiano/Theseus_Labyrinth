#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "model.h"
#include "shader_m.h"
#include "config.h"

// Forward declaration
void PlayerTakeDamage(float damage);

class Enemy {
public:
    // Aggiunto il nuovo stato DORMANT
    enum class State { DORMANT, IDLE, CHASING, ATTACKING, DEATH };

    Model& enemyModel;
    glm::vec3 position;
    glm::vec3 spawnPosition;
    float health;
    float orientation;
    bool isAlive;
    float collisionRadius;
    State currentState;
    glm::vec4 patrolBounds;
    glm::vec3 targetPatrolPoint;
    float stateTimer;
    bool didDealDamage;

    const float ATTACK_RANGE = 2.5f;
    const float NOTICE_RANGE = 12.0f;
    const float ATTACK_DURATION = 1.2f;

    Enemy(Model& model, glm::vec3 startPos, glm::vec4 bounds)
        : enemyModel(model), position(startPos), spawnPosition(startPos), patrolBounds(bounds),
        health(2000.0f), orientation(0.0f), isAlive(true), collisionRadius(0.6f),
        currentState(State::DORMANT), // I nemici ora partono in stato dormiente
        stateTimer(0.0f), didDealDamage(false)
    {
        position.y = 0.2f;
        GetNewPatrolPoint();
    }

    void Activate() {
        if (currentState == State::DORMANT) {
            currentState = State::IDLE;
        }
    }

    void TakeDamage(float damage) {
        if (!isAlive || currentState == State::DORMANT) return;
        health -= damage;
        if (health <= 0) {
            health = 0;
            isAlive = false;
            currentState = State::DEATH;
        }
    }

    void Update(float deltaTime, glm::vec3 playerPosition) {
        if (currentState == State::DORMANT || !isAlive) {
            return; // Se è dormiente o morto, non fare nulla
        }

        float distanceToPlayer = glm::distance(position, playerPosition);

        if (currentState == State::ATTACKING) {
            stateTimer += deltaTime;
            orientation = glm::degrees(atan2(playerPosition.x - position.x, playerPosition.z - position.z));
            if (stateTimer > ATTACK_DURATION / 2.0f && !didDealDamage) {
                if (distanceToPlayer < ATTACK_RANGE + 0.5f) PlayerTakeDamage(10.0f);
                didDealDamage = true;
            }
            if (stateTimer > ATTACK_DURATION) {
                currentState = State::CHASING;
                stateTimer = 0.0f;
            }
            return;
        }

        if (distanceToPlayer < ATTACK_RANGE) {
            currentState = State::ATTACKING;
            stateTimer = 0.0f;
            didDealDamage = false;
        }
        else if (distanceToPlayer < NOTICE_RANGE) {
            currentState = State::CHASING;
        }
        else {
            currentState = State::IDLE;
        }

        if (currentState == State::CHASING) {
            glm::vec3 direction = glm::normalize(playerPosition - position);
            direction.y = 0;
            glm::vec3 nextPos = position + direction * 1.5f * deltaTime;

            if (nextPos.x > patrolBounds.x && nextPos.x < patrolBounds.y &&
                nextPos.z > patrolBounds.z && nextPos.z < patrolBounds.w) {
                position = nextPos;
            }
            orientation = glm::degrees(atan2(direction.x, direction.z));
        }
        else if (currentState == State::IDLE) {
            if (glm::distance(position, targetPatrolPoint) < 1.0f) {
                GetNewPatrolPoint();
            }
            glm::vec3 direction = glm::normalize(targetPatrolPoint - position);
            position += direction * 1.0f * deltaTime;
            orientation = glm::degrees(atan2(direction.x, direction.z));
        }
    }

    void Draw(Shader& shader) {
        if (currentState == State::DORMANT || !isAlive) return; // Se è dormiente o morto, non viene disegnato

        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(orientation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.20f));
        shader.setMat4("model", model);
        enemyModel.Draw(shader);
    }

    void Reset(glm::vec3 startPos) {
        health = 2000.0f;
        isAlive = true;
        position = startPos;
        spawnPosition = startPos;
        currentState = State::DORMANT; // Al reset, torna dormiente
        GetNewPatrolPoint();
    }

private:
    void GetNewPatrolPoint() {
        float x = patrolBounds.x + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (patrolBounds.y - patrolBounds.x)));
        float z = patrolBounds.z + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (patrolBounds.w - patrolBounds.z)));
        targetPatrolPoint = glm::vec3(x, position.y, z);
    }
};

#endif // ENEMY_H