#pragma once
#ifndef TIMER_H
#define TIMER_H

class Timer {
private:
    float m_duration;
    float m_currentTime;

public:
    // Il costruttore imposta la durata del timer
    Timer(float duration_seconds = 1.0f) {
        m_duration = duration_seconds;
        m_currentTime = 0.0f; // Il timer parte inattivo
    }

    // Fa partire il conto alla rovescia
    void Start() {
        m_currentTime = m_duration;
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
};

#endif // TIMER_H