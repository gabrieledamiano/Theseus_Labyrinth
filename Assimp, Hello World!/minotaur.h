#ifndef MINOTAUR_H
#define MINOTAUR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "shader_m.h"
#include "model.h"
#include "animator.h"

class Minotaur {
public:
    // --- MODIFICA: Aggiunto lo stato ATTACKING ---
    enum class State {
        IDLE,
        WALKING,
        ATTACKING,
        GET_HIT,
        DEATH
    };

    Model& minotaurModel;
    Animator animator;

    glm::vec3 position;
    float orientation;
    float speed;
    float health;

    State currentState;
    float stateTimer;

    // Aggiungiamo costanti per la configurazione dell'IA
    const float ATTACK_RANGE = 2.5f;       // Distanza a cui inizia ad attaccare
    const float NOTICE_RANGE = 15.0f;      // Distanza a cui si accorge del giocatore
    const float ATTACK_ANIM_DURATION = 1.5f; // Durata dell'animazione di attacco (da aggiustare)

    Minotaur(Model& model, glm::vec3 startPos)
        : minotaurModel(model),
        animator(model.m_Animations["idle"]),
        position(startPos),
        orientation(0.0f),
        speed(1.8f),
        health(100.0f),
        currentState(State::IDLE),
        stateTimer(0.0f)
    {
    }

    void SetState(State newState)
    {
        if (currentState == newState || currentState == State::DEATH) return;

        currentState = newState;
        stateTimer = 0.0f;

        // Cambia l'animazione nell'animator in base al nuovo stato
        switch (newState)
        {
        case State::IDLE:
            animator.PlayAnimation(minotaurModel.m_Animations["idle"]);
            break;
        case State::WALKING:
            animator.PlayAnimation(minotaurModel.m_Animations["walk"]);
            break;
            // --- MODIFICA: Gestione animazione di attacco ---
        case State::ATTACKING:
            animator.PlayAnimation(minotaurModel.m_Animations["attack"]);
            break;
        case State::GET_HIT:
            animator.PlayAnimation(minotaurModel.m_Animations["get_hit"]);
            break;
        case State::DEATH:
            animator.PlayAnimation(minotaurModel.m_Animations["death"]);
            break;
        }
    }

    void TakeDamage(float damage)
    {
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

        // Macchina a Stati Finiti (FSM)
        switch (currentState)
        {
        case State::IDLE:
            // Se il giocatore si avvicina, inizia a camminare
            if (glm::distance(playerPosition, position) < NOTICE_RANGE) {
                SetState(State::WALKING);
            }
            break;

        case State::WALKING:
        {
            // Se il giocatore è abbastanza vicino, attacca
            if (glm::distance(playerPosition, position) < ATTACK_RANGE) {
                SetState(State::ATTACKING);
                break;
            }

            // Se il giocatore è troppo lontano, torna in idle
            if (glm::distance(playerPosition, position) > NOTICE_RANGE + 1.0f) {
                SetState(State::IDLE);
                break;
            }

            // Logica di movimento verso il giocatore
            glm::vec3 direction = playerPosition - position;
            direction.y = 0;
            direction = glm::normalize(direction);
            position += direction * speed * deltaTime;
            orientation = glm::degrees(atan2(direction.x, direction.z));
            break;
        }

        // --- MODIFICA: Logica per lo stato di attacco ---
        case State::ATTACKING:
            stateTimer += deltaTime;
            // Orienta il minotauro verso il giocatore prima dell'attacco
            orientation = glm::degrees(atan2(playerPosition.x - position.x, playerPosition.z - position.z));

            // Qui, a metà animazione, potresti infliggere il danno al giocatore
            // if(stateTimer > ATTACK_ANIM_DURATION / 2.0f) { /* infliggi danno */ }

            // Finita l'animazione, torna a inseguire il giocatore
            if (stateTimer > ATTACK_ANIM_DURATION) {
                SetState(State::WALKING);
            }
            break;

        case State::GET_HIT:
            stateTimer += deltaTime;
            if (stateTimer > 0.5f) { // Durata dell'animazione "colpito"
                SetState(State::WALKING);
            }
            break;

        case State::DEATH:
            // Rimane a terra, non fa nulla
            break;
        }
    }

    void Draw(Shader& shader) {
        shader.use();

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            shader.setMat4("finalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.015f));
        model = glm::rotate(model, glm::radians(orientation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        shader.setMat4("model", model);
        minotaurModel.Draw(shader);
    }
};

#endif