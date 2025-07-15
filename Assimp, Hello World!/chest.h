#pragma once
#ifndef CHEST_H
#define CHEST_H

#include <glm/glm.hpp>
#include <string>
#include "model.h"
#include "shader_m.h"

// Definiamo i tipi di potenziamenti possibili
enum class PowerUpType {
    NONE,
    HEALTH_BOOST,   // Nettare degli Dei
    DAMAGE_BOOST    // Furia di Ares
};

class Chest {
public:
    Model& chestModel;
    glm::vec3 position;
    bool isOpen;
    PowerUpType powerUp;

    Chest(Model& model, glm::vec3 pos)
        : chestModel(model), position(pos), isOpen(false), powerUp(PowerUpType::NONE)
    {
        // Assegna un power-up casuale quando la cassa viene creata
        int randomValue = rand() % 2; // 0 o 1
        if (randomValue == 0) {
            powerUp = PowerUpType::HEALTH_BOOST;
        }
        else {
            powerUp = PowerUpType::DAMAGE_BOOST;
        }
    }

    // Funzione per aprire la cassa
    void Open() {
        if (!isOpen) {
            isOpen = true;
            // In futuro, qui potresti anche avviare un'animazione di apertura
            std::cout << "Cassa aperta! Contiene: " << GetPowerUpName() << std::endl;
        }
    }

    // Funzione helper per ottenere il nome del power-up
    std::string GetPowerUpName() const {
        switch (powerUp) {
        case PowerUpType::HEALTH_BOOST: return "Nettare degli Dei";
        case PowerUpType::DAMAGE_BOOST: return "Furia di Ares";
        default: return "Niente";
        }
    }

    void Draw(Shader& shader) {
        // Non disegniamo la cassa se è già stata aperta
        if (isOpen) return;

        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.3f)); // Scala la cassa se necessario
        shader.setMat4("model", model);
        chestModel.Draw(shader);
    }
};

#endif // CHEST_H