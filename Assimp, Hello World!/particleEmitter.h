#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader_m.h"
#include "particle.h"

class ParticleEmitter {
public:
    ParticleEmitter(unsigned int amount);

    void Update(float dt, const glm::vec3& objectPosition, unsigned int newParticles);
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    std::vector<Particle> particles;
    unsigned int amount;
    unsigned int VAO;
    unsigned int VBO;
    Shader particleShader;

    void init();
    unsigned int firstUnusedParticle();
    void respawnParticle(Particle& particle, const glm::vec3& objectPosition);
};
