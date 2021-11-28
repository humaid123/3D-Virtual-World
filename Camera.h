#ifndef CAMERA_H
#define CAMERA_H

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"
#include <math.h>

using namespace OpenGP;

class Camera {
public:
    Vec3 cameraPos, cameraFront, cameraUp;

    float fov, speed;
    const float speedIncrement = 0.005f, mouseSensitivity = 0.005f;

    // used for typical mouse control
    // yaw controls the side movement, yaw = 0 means we are looking forward, yaw = 90 means we are looking east, yaw = 270 means we are looking west
    // pitch controls the up-down movement pitch = 0 means we are looking forward, pitch = 90 means we are looking up, pitch = 270 means we are looking down
    float yaw, pitch;

    float nearPlane =0.1f, farPlane = 60.0f, aspectRatio;
public:
    Camera(int _width, int _height) {
        cameraPos = Vec3(0.0f, 0.0f, 3.0f);
        cameraFront = Vec3(0.0f, -1.0f, 0.0f);
        cameraUp = Vec3(0.0f, 0.0f, 1.0f);
        fov = 80.0f;
        speed = 0.01f;
        yaw = 0.0f;
        pitch = 0.0f;
        aspectRatio = (float)_width / (float) _height;
    }

    void updateYawAndPitch(Vec2 delta) {
        delta[1] = -delta[1];
        delta = mouseSensitivity * delta;

        yaw += delta[0];
        pitch += delta[1];

        if(pitch > M_PI/2.0f - 0.01f)  pitch =  M_PI/2.0f - 0.01f;
        if(pitch <  -M_PI/2.0f + 0.01f) pitch =  -M_PI/2.0f + 0.01f;

        cameraFront = Vec3( 
            sin(yaw) * cos(pitch),
            cos(yaw) * cos(pitch),
            sin(pitch)
        );
    }

    void invertPitch() {
        pitch = -pitch;

        cameraFront = Vec3(
            sin(yaw) * cos(pitch),
            cos(yaw) * cos(pitch),
            sin(pitch)
        );
    }

    void updateCamera(KeyEvent k) {
        // WASD
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

        // arrow movements => up decreases the fov, right increases the speed
        if (k.key == GLFW_KEY_UP) {
            fov = std::max(1.0f, fov-1.0f);
        }

        if (k.key == GLFW_KEY_DOWN) {
            fov = std::min(80.0f, fov + 1);
        }

        if (k.key == GLFW_KEY_RIGHT) {
            speed = std::min(1.0f, speed + speedIncrement);
        }

        if (k.key == GLFW_KEY_LEFT) {
            speed = std::max(0.001f, speed - speedIncrement);
        }
    }

    Mat4x4 viewMatrix() {
        Vec3 look = cameraFront + cameraPos;
        return lookAt(cameraPos, look, cameraUp);
    }

    Mat4x4 projectionMatrix() {
        return perspective(fov, aspectRatio, nearPlane, farPlane);
    }
};


#endif