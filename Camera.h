#ifndef CAMERA_H
#define CAMERA_H

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"
using namespace OpenGP;
constexpr float PI = 3.14159265359f;

class Camera {
public:
Vec3 cameraPos;
Vec3 cameraFront;
Vec3 cameraUp;
float fov;
float speed;
float speedIncrement;
float halflife;
Vec3 displacement;

float yaw;
float pitch;
float width, height;
public:
    Camera(int _width, int _height) {
        // Initialize camera position and direction
        cameraPos = Vec3(0.0f, 0.0f, /*10.0f*/3.0f);
        cameraFront = Vec3(0.0f, -1.0f, 0.0f);
        cameraUp = Vec3(0.0f, 0.0f, 1.0f);

        // Initialize FOV and camera speed
        fov = 80.0f;
        speed = 0.01f;
        speedIncrement = 0.002f;

        // Initialize yaw (left/right) and pitch (up/down) angles
        yaw = 0.0f;
        pitch = 0.0f;

        width = _width;
        height = _height;
    }

    void updateYawAndPitch(Vec2 delta) {
        delta[1] = -delta[1];
        float sensitivity = 0.005f;
        delta = sensitivity * delta;

        yaw += delta[0];
        pitch += delta[1];

        if(pitch > PI/2.0f - 0.01f)  pitch =  PI/2.0f - 0.01f;
        if(pitch <  -PI/2.0f + 0.01f) pitch =  -PI/2.0f + 0.01f;

        Vec3 front(0, 0, 0);
        front[0] = sin(yaw) * cos(pitch);
        front[1] = cos(yaw) * cos(pitch);
        front[2] = sin(pitch);

        cameraFront = front.normalized();
    }

    void invertPitch() {
        pitch = -pitch;

        if (pitch > PI / 2.0f - 0.01f)  pitch = PI / 2.0f - 0.01f;
        if (pitch < -PI / 2.0f + 0.01f) pitch = -PI / 2.0f + 0.01f;

        Vec3 front(0, 0, 0);
        front[0] = sin(yaw) * cos(pitch);
        front[1] = cos(yaw) * cos(pitch);
        front[2] = sin(pitch);

        cameraFront = front.normalized();
    }

    void updateCamera(KeyEvent k) {
        // Movement left, right, up and down (WASD)
        if (k.key == GLFW_KEY_W) {
            cameraPos = cameraPos + speed * cameraFront.normalized();
        }

        if (k.key == GLFW_KEY_A) {
            cameraPos = cameraPos - speed * cameraFront.normalized().cross(cameraUp);
        }

        if (k.key == GLFW_KEY_S) {
            cameraPos = cameraPos - speed * cameraFront.normalized();
        }

        if (k.key == GLFW_KEY_D) {
            cameraPos = cameraPos + speed * cameraFront.normalized().cross(cameraUp);
        }

        // Adjust FOV
        if (k.key == GLFW_KEY_UP) {
            fov -= 1.0f;
            if (fov <= 1.0f) fov = 1.0f;
        }

        if (k.key == GLFW_KEY_DOWN) {
            fov += 1.0f;
            if (fov >= 80.0f) fov = 80.0f;
        }

        // Adjust movement speed
        if (k.key == GLFW_KEY_RIGHT) {
            speed += speedIncrement;
            if (speed >= 1.0f) speed = 1.0f;
        }

        if (k.key == GLFW_KEY_LEFT) {
            speed -= speedIncrement;
            if (speed <= 0.001f) speed = 0.001f;
        }
    }

    Mat4x4 viewMatrix() {
        Vec3 look = cameraFront + cameraPos;
        Mat4x4 V = lookAt(cameraPos, look, cameraUp);
        return V;
    }

    Mat4x4 projectionMatrix() {
        Mat4x4 P = perspective(fov, (float) width / (float)height, 0.1f, 60.0f);
        return P;
    }
};


#endif