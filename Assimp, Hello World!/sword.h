#ifndef SWORD_H
#define SWORD_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_m.h"
#include "model.h"
#include "camera.h"

class Sword {
public:
    Model swordModel;
    bool isAttacking;
    float attackTimer;
    const float ATTACK_DURATION = 0.4f;

    Sword(const std::string& path)
        : swordModel(path), isAttacking(false), attackTimer(0.0f)
    {
    }

    void Attack() {
        if (!isAttacking) {
            isAttacking = true;
            attackTimer = 0.0f;
        }
    }

    void Update(float deltaTime) {
        if (isAttacking) {
            attackTimer += deltaTime;
            if (attackTimer >= ATTACK_DURATION) {
                isAttacking = false;
                attackTimer = 0.0f;
            }
        }
    }

    // --- NUOVA FUNZIONE DRAW BASATA SULLA LOGICA DEL FUCILE ---
    void Draw(Shader& shader, Camera& camera, const glm::mat4& projection, const glm::mat4& view) {

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view); 

        // 1. Calcola la posizione della spada nel mondo, relativa alla camera
        glm::vec3 swordOffset = glm::vec3(0.3f, -0.4f, 0.5f); // (destra, basso, avanti)
        glm::vec3 swordPosition = camera.Position + (camera.Right * swordOffset.x) + (camera.Up * swordOffset.y) + (camera.Front * swordOffset.z);

        // 2. Costruisci la matrice del modello
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, swordPosition);

        // 3. Applica una rotazione di base per orientare la spada
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // 4. ANNULLA la rotazione della camera per "incollarla" alla visuale
        // Questo è il trucco principale di questa tecnica
        model = glm::rotate(model, glm::radians(camera.Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f));

        // 5. Se sta attaccando, applica l'animazione di swing
        if (isAttacking) {
            float progress = attackTimer / ATTACK_DURATION;
            float swingAngle = sin(progress * 3.14159f) * 45.0f;
            model = glm::rotate(model, glm::radians(-swingAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        // 6. Applica la scala finale
        model = glm::scale(model, glm::vec3(1.1f));

        shader.setMat4("model", model);
        swordModel.Draw(shader);
    }
};

#endif