#pragma once
#include "shader_m.h"
#include "model.h"

class WallSconce {
public:
    Model& model;
    glm::vec3 position;
    float rotationAngle;
    float scale;
    glm::vec3 flameOffset; 

    WallSconce(Model& model, glm::vec3 pos, float rotation, float scl, glm::vec3 offset)
        : model(model), position(pos), rotationAngle(rotation), scale(scl), flameOffset(offset) {} 

    void Draw(Shader& shader) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        shader.setMat4("model", modelMatrix);
        model.Draw(shader);
    }
};