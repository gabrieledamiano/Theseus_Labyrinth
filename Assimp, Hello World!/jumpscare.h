#pragma once
#pragma once
#ifndef JUMPSCARE_H
#define JUMPSCARE_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "model.h"
#include "shader_m.h"
#include "timer.h"
#include "camera.h"

class JumpScare {
public:
    enum class State { INACTIVE, APPEARING, ACTIVE, DISAPPEARING };

    Model& scareModel;
    State currentState;
    glm::vec3 position;
    Timer stateTimer;

    JumpScare(Model& model)
        : scareModel(model), currentState(State::INACTIVE), stateTimer(1.0f) {
    }

    void Trigger(glm::vec3 pos) {
        if (currentState == State::INACTIVE) {
            position = pos;
            // Regola l'altezza per essere al livello degli occhi del giocatore
            position.y = pos.y - 0.5f; // Leggermente più in basso per essere più minacciosa
            currentState = State::APPEARING;
            stateTimer.Start(0.3f); // Appare più velocemente per effetto shock

            // DEBUG: Stampa la posizione di spawn
            std::cout << "JumpScare triggered at: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        }
    }

    void Update(float deltaTime) {
        if (currentState == State::INACTIVE) return;

        stateTimer.Update(deltaTime);

        if (!stateTimer.IsActive()) {
            if (currentState == State::APPEARING) {
                currentState = State::ACTIVE;
                stateTimer.Start(0.5f); // Rimane visibile più a lungo
                std::cout << "JumpScare now ACTIVE" << std::endl;
            }
            else if (currentState == State::ACTIVE) {
                currentState = State::DISAPPEARING;
                stateTimer.Start(0.8f); // Scompare più lentamente
                std::cout << "JumpScare DISAPPEARING" << std::endl;
            }
            else if (currentState == State::DISAPPEARING) {
                currentState = State::INACTIVE;
                std::cout << "JumpScare INACTIVE" << std::endl;
            }
        }
    }

    void Draw(Shader& shader, const Camera& camera, const glm::mat4& projection, const glm::mat4& view) {
        if (currentState == State::INACTIVE) return;

        // Calcola l'alpha in modo più semplice e visibile
        float alpha = 1.0f;
        if (currentState == State::APPEARING) {
            float progress = 1.0f - (stateTimer.GetTime() / stateTimer.GetStartTime());
            alpha = progress; // Da 0 a 1
        }
        else if (currentState == State::ACTIVE) {
            alpha = 1.0f; // Completamente visibile
        }
        else if (currentState == State::DISAPPEARING) {
            float progress = stateTimer.GetTime() / stateTimer.GetStartTime();
            alpha = 1.0f - progress; // Da 1 a 0
        }

        // Assicurati che l'alpha non sia mai 0 durante il rendering
        alpha = glm::max(alpha, 0.4f);

        shader.use();
        shader.setFloat("alpha", alpha);


        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);

        // Fai sempre guardare l'arpia verso il giocatore
        glm::vec3 direction = glm::normalize(camera.Position - position);
        float yaw = glm::degrees(atan2(direction.x, direction.z));
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

        // Scala più grande per renderla più visibile
        model = glm::scale(model, glm::vec3(2.5f)); // Aumentato da 1.5f a 2.5f

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);


        scareModel.Draw(shader);
    }
};

#endif // JUMPSCARE_H
