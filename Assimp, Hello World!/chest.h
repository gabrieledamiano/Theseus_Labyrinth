#pragma once
#ifndef CHEST_H
#define CHEST_H

#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include "model.h"
#include "shader_m.h"

// Tipi di potenziamenti possibili
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
    bool isLocked; // true se non può essere aperta

    PowerUpType powerUp;

    Chest(Model& model, glm::vec3 pos)
        : chestModel(model), position(pos), isOpen(false), isLocked(true), powerUp(PowerUpType::NONE)
    {
        // Assegna un power-up casuale
        if ((rand() % 2) == 0) {
            powerUp = PowerUpType::HEALTH_BOOST;
        }
        else {
            powerUp = PowerUpType::DAMAGE_BOOST;
        }
    }

    void Open() {
        if (isLocked) {
            std::cout << "La cassa e' bloccata! Sconfiggi i guardiani." << std::endl;
            return;
        }
        if (!isOpen) {
            isOpen = true;
            std::cout << "Cassa aperta! Contiene: " << GetPowerUpName() << std::endl;
        }
    }

    std::string GetPowerUpName() const {
        switch (powerUp) {
        case PowerUpType::HEALTH_BOOST: return "Nettare degli Dei";
        case PowerUpType::DAMAGE_BOOST: return "Furia di Ares";
        default: return "Niente";
        }
    }

    void Reset() {
        isOpen = false;
        isLocked = true;
    }

    void Draw(Shader& shader) {
        if (isOpen) return;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f));
        shader.setMat4("model", model);
        chestModel.Draw(shader);
    }
};

#endif // CHEST_H