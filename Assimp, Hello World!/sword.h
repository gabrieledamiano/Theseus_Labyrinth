#ifndef SWORD_H
#define SWORD_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// In cima a sword.h
#include <glm/gtx/string_cast.hpp>
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
        : swordModel(path), isAttacking(false), attackTimer(0.0f) {
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

    void Draw(Shader& shader, const Camera& camera) {
        // Pulisce il depth buffer per disegnare la spada sempre in primo piano
        glClear(GL_DEPTH_BUFFER_BIT);

        shader.use();

        // --- LA MAGIA DELLA PRIMA PERSONA ---

        // 1. Matrice di Proiezione: usiamo un FOV fisso per l'arma, così non si deforma quando usi lo zoom.
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

        // 2. Matrice di Vista: questa è la parte cruciale.
        // Prendiamo la matrice di vista della camera e ne annulliamo la parte di traslazione (posizione).
        // In questo modo, la spada eredita solo la ROTAZIONE della camera.
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        // 3. Matrice del Modello: questa matrice ora posiziona la spada RELATIVAMENTE allo schermo, non al mondo.
        glm::mat4 model = glm::mat4(1.0f);

        // Prima trasliamo per posizionarla (es. in basso a destra)
        model = glm::translate(model, glm::vec3(0.6f, -0.6f, -1.0f));

        // Poi applichiamo le rotazioni di base per orientarla correttamente
        model = glm::rotate(model, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // 4. Se sta attaccando, applichiamo un'ulteriore rotazione per l'animazione
        if (isAttacking) {
            float progress = attackTimer / ATTACK_DURATION;
            float swingAngle = sin(progress * 3.14159f) * 60.0f;
            model = glm::rotate(model, glm::radians(swingAngle), glm::vec3(1.0f, 0.0f, -0.5f));
        }

        // Infine, applichiamo la scala
        model = glm::scale(model, glm::vec3(1.0f));

        // Inviamo le matrici allo shader
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);

        swordModel.Draw(shader);
    }
};

#endif