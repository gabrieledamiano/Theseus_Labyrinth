#pragma once
#ifndef DUNGEON_ROOM_H
#define DUNGEON_ROOM_H

#include <vector>
#include <iostream>
#include "chest.h"
#include "enemy.h"

class DungeonRoom {
public:
    enum class RoomState { IDLE, ACTIVE, CLEARED };

    Chest chest;
    std::vector<Enemy> enemies;
    RoomState state;
    glm::vec4 bounds;
    bool firstActivation;

private:
    // NUOVA VARIABILE: Memorizza le posizioni di spawn originali dei nemici
    std::vector<glm::vec3> m_enemySpawnPositions;

public:
    DungeonRoom(Chest c, std::vector<Enemy> e, glm::vec4 b)
        : chest(c), enemies(std::move(e)), bounds(b), state(RoomState::IDLE), firstActivation(true)
    {
        // All'atto della creazione, salviamo le posizioni iniziali dei nemici
        for (const auto& enemy : enemies) {
            m_enemySpawnPositions.push_back(enemy.spawnPosition);
        }
    }

    void Update(float deltaTime, glm::vec3 playerPosition) {
        float playerX = playerPosition.x;
        float playerZ = playerPosition.z;

        if (state == RoomState::IDLE && firstActivation &&
            playerX > bounds.x && playerX < bounds.y &&
            playerZ > bounds.z && playerZ < bounds.w) {
            state = RoomState::ACTIVE;
            std::cout << "Sei entrato in un'area sorvegliata!" << std::endl;
        }

        if (state == RoomState::ACTIVE) {
            bool allEnemiesDefeated = true;
            for (auto& enemy : enemies) {
                if (enemy.health > 0) {
                    enemy.Update(deltaTime, playerPosition);
                    allEnemiesDefeated = false;
                }
            }

            if (allEnemiesDefeated) {
                state = RoomState::CLEARED;
                chest.isLocked = false;
                std::cout << "Guardiani sconfitti! La cassa e' ora accessibile." << std::endl;
            }
        }
    }

    // --- FUNZIONE RESET CORRETTA ---
    void Reset() {
        state = RoomState::IDLE;
        firstActivation = true;
        chest.Reset();

        // Ora usiamo le posizioni salvate per resettare ogni nemico
        for (size_t i = 0; i < enemies.size() && i < m_enemySpawnPositions.size(); ++i) {
            enemies[i].Reset(m_enemySpawnPositions[i]);
        }
    }
};

#endif // DUNGEON_ROOM_H