#pragma once
#ifndef TIMER_H
#define TIMER_H

class Timer {
private:
    float m_startTime;      // Durata originale o ultima impostata
    float m_currentTime;    // Tempo rimanente

public:
    Timer(float startTime_seconds = 1.0f)
        : m_startTime(startTime_seconds), m_currentTime(0.0f) {
    }

    // Fa partire il conto alla rovescia con la durata predefinita
    void Start() {
        m_currentTime = m_startTime;
    }

    
    // Fa partire il conto alla rovescia con una NUOVA durata personalizzata
    void Start(float new_duration) {
        m_startTime = new_duration; // Aggiorna la durata di riferimento
        m_currentTime = new_duration;
    }

    void Stop() {
        m_currentTime = 0.0f;
    }

    void Update(float deltaTime) {
        if (m_currentTime > 0.0f) {
            m_currentTime -= deltaTime;
            if (m_currentTime < 0.0f) m_currentTime = 0.0f;
        }
    }

    bool IsActive() const {
        return m_currentTime > 0.0f;
    }

    // Ritorna il tempo rimanente
    float GetTime() const {
        return m_currentTime;
    }

    // Ritorna la durata con cui è stato avviato l'ultimo timer
    float GetStartTime() const {
        return m_startTime;
    }
};

#endif // TIMER_H