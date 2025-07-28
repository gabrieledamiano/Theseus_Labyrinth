#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// PER I PIANI DEL FRUSTUM 
struct FrustumPlane {
    glm::vec3 normal;
    float distance;
};

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    std::vector<FrustumPlane> frustum;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
        : Position(position), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)), Yaw(YAW), Pitch(PITCH),
        Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void Reset() {
        Position = glm::vec3(1.5f * 2.0f, 1.7f, 1.5f * 2.0f);
        Yaw = YAW;
        Pitch = PITCH;
        updateCameraVectors();
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool isSprinting) {
        float velocity = (isSprinting ? MovementSpeed * 1.8f : MovementSpeed) * deltaTime;
        if (direction == FORWARD) Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT) Position -= Right * velocity;
        if (direction == RIGHT) Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        Yaw += xoffset * MouseSensitivity;
        Pitch += yoffset * MouseSensitivity;
        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    //  PER ESTRARRE I PIANI DEL FRUSTUM 
    void UpdateFrustum(const glm::mat4& view, const glm::mat4& projection) {
        frustum.clear();
        glm::mat4 clipMatrix = projection * view;

        // Estrae i 6 piani (destra, sinistra, basso, alto, vicino, lontano)
        for (int i = 0; i < 2; ++i) {
            // Piano Sinistra e Destra
            FrustumPlane plane;
            plane.normal.x = clipMatrix[0][3] + (i == 0 ? clipMatrix[0][0] : -clipMatrix[0][0]);
            plane.normal.y = clipMatrix[1][3] + (i == 0 ? clipMatrix[1][0] : -clipMatrix[1][0]);
            plane.normal.z = clipMatrix[2][3] + (i == 0 ? clipMatrix[2][0] : -clipMatrix[2][0]);
            plane.distance = clipMatrix[3][3] + (i == 0 ? clipMatrix[3][0] : -clipMatrix[3][0]);
            frustum.push_back(plane);

            // Piano Basso e Alto
            plane.normal.x = clipMatrix[0][3] + (i == 0 ? clipMatrix[0][1] : -clipMatrix[0][1]);
            plane.normal.y = clipMatrix[1][3] + (i == 0 ? clipMatrix[1][1] : -clipMatrix[1][1]);
            plane.normal.z = clipMatrix[2][3] + (i == 0 ? clipMatrix[2][1] : -clipMatrix[2][1]);
            plane.distance = clipMatrix[3][3] + (i == 0 ? clipMatrix[3][1] : -clipMatrix[3][1]);
            frustum.push_back(plane);

            // Piano Vicino e Lontano
            plane.normal.x = clipMatrix[0][3] + (i == 0 ? clipMatrix[0][2] : -clipMatrix[0][2]);
            plane.normal.y = clipMatrix[1][3] + (i == 0 ? clipMatrix[1][2] : -clipMatrix[1][2]);
            plane.normal.z = clipMatrix[2][3] + (i == 0 ? clipMatrix[2][2] : -clipMatrix[2][2]);
            plane.distance = clipMatrix[3][3] + (i == 0 ? clipMatrix[3][2] : -clipMatrix[3][2]);
            frustum.push_back(plane);
        }

        // Normalizza i piani
        for (auto& plane : frustum) {
            float length = glm::length(plane.normal);
            plane.normal /= length;
            plane.distance /= length;
        }
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif