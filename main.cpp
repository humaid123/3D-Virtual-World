#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"
#include <iostream>

#include "loadTexture.h"

#include "Terrain.h"
#include "Skybox.h"
#include "Camera.h"
#include "Water.h"

using namespace OpenGP;
const int width=1280, height=720;

int main(int, char**){

    Application app;

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);     // Make skybox seamless
    glEnable(GL_DEPTH_TEST);     // enable depth test for skybox and so on
    glEnable(GL_CLIP_DISTANCE0); // enables the first clipping plane in the program => shaders can now use gl_ClipDistance[0]

    float waterHeight = 0.5f;
    Vec3 skyColor = Vec3(0.6, 0.7, 0.8);
    Vec3 lightPos = Vec3(10.0f, 10.0f, 10.0f);
    float size_grid_x = 20, size_grid_y = 20; // make grid bigger [-10, 10] instead of [-1, 1] to hide rendering distance.


    Skybox skybox(skyColor);
    Water water(size_grid_x, size_grid_y, waterHeight);
    Terrain terrain(size_grid_x, size_grid_y, waterHeight, skyColor, lightPos);
    Camera camera(width, height);

    // clipping plane => required when shading reflection and refraction FBO
    Vec3 clipPlaneNormal = Vec3(0, 0, -1); // Vec3(0, -1, 0);
    float clipPlaneHeight = 1000;
    Vec3 reflectionClipPlaneNormal = Vec3(0, 0, 1); // Vec3(0, 1, 0);
    float reflectionClipPlaneHeight = -waterHeight;
    Vec3 refractionClipPlaneNormal = Vec3(0, 0, -1); //Vec3(0, -1, 0);
    float refractionClipPlaneHeight = waterHeight;


    // Display callback
    Window& window = app.create_window([&](Window&){
        float time = glfwGetTime();

        // for reflection, we draw the whole scene on the FBO from a camera that is position below the current eye position and pointing upwards
        // essentially we get angle of incidence = angle of reflection
        glViewport(0, 0, water.reflectionWidth, water.reflectionHeight);
        water.reflectionFBO->bind();
        // to get the reflection, the camera needs to move down and point upwards => move by current distance * 2 down and flip
        // this draws the reflection into the texture
        float distance = 2 * (camera.cameraPos.z() - waterHeight);
        camera.cameraPos.z() -= distance;
        camera.invertPitch();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            skybox.draw(camera, time);
            terrain.draw(camera, reflectionClipPlaneNormal, reflectionClipPlaneHeight);
        water.reflectionFBO->unbind();
        camera.cameraPos.z() += distance;
        camera.invertPitch();

        // for refraction, we draw the scene below the water height => we will blend this with 
        // the reflection texture to create a water effect
        glViewport(0, 0, water.refractionWidth, water.refractionHeight);
        water.refractionFBO->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            terrain.draw(camera, refractionClipPlaneNormal, refractionClipPlaneHeight);
        water.refractionFBO->unbind();

        // actual drawing
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        terrain.draw(camera, clipPlaneNormal, clipPlaneHeight);
        skybox.draw(camera, time);
        water.draw(camera, time);
    });
    window.set_title("3D-Virtual-World");
    window.set_size(width, height);


    // Handle mouse input (looking around the screen)
    Vec2 mouse(0,0);
    window.add_listener<MouseMoveEvent>([&](const MouseMoveEvent &m){
        // Camera control
        Vec2 delta = m.position - mouse;
        camera.updateYawAndPitch(delta);
        mouse = m.position;
    });

    // Handle keyboard input (moving around the screen)
    window.add_listener<KeyEvent>([&](const KeyEvent &k){
        camera.updateCamera(k);
    });

    return app.run();
}