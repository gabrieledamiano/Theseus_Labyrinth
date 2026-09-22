// CHEST.H
#pragma once
#ifndef CHEST_H
#define CHEST_H

#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include "model.h"
#include "shader_m.h"

enum class PowerUpType {
    NONE,
    HEALTH_BOOST,
    DAMAGE_BOOST
};

class Chest {
public:
    Model& chestModel;
    glm::vec3 position;
    bool isOpen;
    bool isLocked;
    bool isCollected; 
    PowerUpType powerUp;

    Chest(Model& model, glm::vec3 pos)
        : chestModel(model), position(pos), isOpen(false), isLocked(true),
        isCollected(false), powerUp(PowerUpType::NONE) // Inizializza isCollected a false
    {
        if ((rand() % 2) == 0) {
            powerUp = PowerUpType::HEALTH_BOOST;
        }
        else {
            powerUp = PowerUpType::DAMAGE_BOOST;
        }
    }

    bool Open() {
        if (isLocked) {
            std::cout << "La cassa è bloccata!" << std::endl;
            return false;
        }

        
        if (isCollected) {
            return false;
        }

        isOpen = true;
        isCollected = true; // IMPOSTA A TRUE DOPO L'APERTURA
        std::cout << "Cassa aperta! Contiene: " << GetPowerUpName() << std::endl;
        return true;
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
        isCollected = false;
    }

    void Draw(Shader& shader) {
        // Non disegnare se la cassa è stata raccolta o aperta
        if (isCollected || isOpen) return;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f));
        shader.setMat4("model", model);
        chestModel.Draw(shader);
    }
};

#endif // CHEST_H