#include "ParticleEmitter.h"
#include <glad/glad.h>
#include <algorithm> 

ParticleEmitter::ParticleEmitter(unsigned int amount)
    : amount(amount), particleShader("particle.vs", "particle.fs") {
    this->init();
}

void ParticleEmitter::init() {
    particles.resize(amount);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Alloca memoria per tutte le particelle. Useremo GL_STREAM_DRAW perché i dati cambiano spesso.
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(Particle), nullptr, GL_STREAM_DRAW);

    // Vertex Attributes (passiamo solo la posizione per ora)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Position));

    glBindVertexArray(0);
}

void ParticleEmitter::Update(float dt, const glm::vec3& objectPosition, unsigned int newParticles) {
    // Aggiungi nuove particelle
    for (unsigned int i = 0; i < newParticles; ++i) {
        int unusedParticle = this->firstUnusedParticle();
        if (unusedParticle >= 0) {
            this->respawnParticle(this->particles[unusedParticle], objectPosition);
        }
    }

    // Aggiorna tutte le particelle esistenti
    for (unsigned int i = 0; i < this->amount; ++i) {
        Particle& p = this->particles[i];
        p.Life -= dt; // Riduci la vita
        if (p.Life > 0.0f) {
            p.Position -= p.Velocity * dt;
            p.Color.a = p.Life; // Usa la vita rimanente per la trasparenza (opzionale)
        }
    }
}

void ParticleEmitter::Draw(const glm::mat4& view, const glm::mat4& projection) {
    // Abilita il blending additivo per l'effetto fuoco
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    particleShader.use();
    particleShader.setMat4("view", view);
    particleShader.setMat4("projection", projection);

    // Aggiorna il VBO con le posizioni delle particelle attive
    std::vector<glm::vec3> positions;
    for (const Particle& p : particles) {
        if (p.Life > 0.0f) {
            positions.push_back(p.Position);
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Aggiorna solo la porzione del buffer che ci serve
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    glEnable(GL_PROGRAM_POINT_SIZE); // Permette di impostare la dimensione dei punti nel vertex shader

    glDrawArrays(GL_POINTS, 0, positions.size());

    glDisable(GL_PROGRAM_POINT_SIZE);
    glBindVertexArray(0);

    // Ripristina il blending standard
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


unsigned int ParticleEmitter::firstUnusedParticle() {
    // Cerca una particella non più attiva
    for (unsigned int i = 0; i < this->amount; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            return i;
        }
    }
    // Altrimenti, sovrascrivi la prima
    return 0;
}

void ParticleEmitter::respawnParticle(Particle& particle, const glm::vec3& objectPosition) {
    float randomX = ((rand() % 100) - 50) / 100.0f;
    float randomZ = ((rand() % 100) - 50) / 100.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);

    particle.Position = objectPosition + glm::vec3(randomX, 0.0, randomZ) * 0.1f;
    particle.Life = 1.0f; // Vive per 1 secondo
    particle.Velocity = glm::vec3(0.0f, -0.8f, 0.0f); // Si muove verso l'alto
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
}