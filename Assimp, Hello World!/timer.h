#pragma once
#ifndef TIMER_H
#define TIMER_H

class Timer {
private:
    const float m_startTime; // Rinominato da m_duration per chiarezza
    float m_currentTime;

public:
    // Il costruttore imposta la durata iniziale del timer
    Timer(float startTime_seconds = 1.0f) : m_startTime(startTime_seconds) {
        m_currentTime = 0.0f; // Il timer parte inattivo
    }

    // Fa partire il conto alla rovescia
    void Start() {
        m_currentTime = m_startTime;
    }

    // Resetta e ferma il timer
    void Stop() {
        m_currentTime = 0.0f;
    }

    // Aggiorna il timer. Va chiamata ad ogni frame.
    void Update(float deltaTime) {
        if (m_currentTime > 0.0f) {
            m_currentTime -= deltaTime;
        }
    }

    // Ritorna 'true' se il timer è ancora in corso
    bool IsActive() const {
        return m_currentTime > 0.0f;
    }

    // --- NUOVE FUNZIONI AGGIUNTE ---

    // Ritorna il tempo rimanente
    float GetTime() const {
        return m_currentTime;
    }

    // Ritorna la durata iniziale
    float GetStartTime() const {
        return m_startTime;
    }
};

#endif // TIMER_H