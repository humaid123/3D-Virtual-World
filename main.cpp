#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"

#include "loadTexture.h"

// new
// #include "ParticleMaster.h"
// new 

#include "Terrain.h"
#include "Skybox.h"
#include "Camera.h"
#include "Water.h"
#include <iostream>


using namespace OpenGP;
const int width=1280, height=720;

int main(int, char**){

    Application app;

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // Enable seamless cubemap
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // enable depth test for skybox and so on
    glEnable(GL_DEPTH_TEST);


    Terrain terrain;
    Skybox skybox;
    
    // new
    // ParticleSystem particleSystem;
    // new
    
    float waterHeight = 0.7f;
    Water water(waterHeight);
    Camera camera(width, height);

    Vec3 clipPlaneNormal = Vec3(0, 0, -1); // Vec3(0, -1, 0);
    float clipPlaneHeight = 1000;


    // REFLECTION IS NOT GETTING CULLED NEED TO TEST THAT....
    Vec3 reflectionClipPlaneNormal = Vec3(0, 0, 1); // Vec3(0, 1, 0);
    float reflectionClipPlaneHeight = -waterHeight;

    Vec3 refractionClipPlaneNormal = Vec3(0, 0, -1); //Vec3(0, -1, 0);
    float refractionClipPlaneHeight = waterHeight;

    // Display callback
    Window& window = app.create_window([&](Window&){
        float time = glfwGetTime();

        // new
        // particleSystem.update(time);
        // particleSystem.emitParticles(camera.cameraPos);
        // new

        glEnable(GL_CLIP_DISTANCE0); // enables the first clipping plane in the program => shaders can now use gl_ClipDistance[0]

        glViewport(0, 0, water.reflectionWidth, water.reflectionHeight);
        water.reflectionFBO->bind();
        // to get the reflection, the camera needs to move down and point upwards => move by current distance * 2 down and flip
        // this draws the reflection into the texture
        float distance = 2 * (camera.cameraPos.z() - waterHeight);
        camera.cameraPos.z() -= distance;
        camera.invertPitch();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox.draw(camera);
        terrain.draw(camera, reflectionClipPlaneNormal, reflectionClipPlaneHeight, waterHeight);
        water.reflectionFBO->unbind();
        camera.cameraPos.z() += distance;
        camera.invertPitch();


        glViewport(0, 0, water.refractionWidth, water.refractionHeight);
        water.refractionFBO->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        terrain.draw(camera, refractionClipPlaneNormal, refractionClipPlaneHeight, waterHeight);
        water.refractionFBO->unbind();

        // actual drawing
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        terrain.draw(camera, clipPlaneNormal, clipPlaneHeight, waterHeight);
        skybox.draw(camera);
        water.draw(camera, time);

        // new 
        // particleSystem.draw(camera);
        // new 
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

//void init(){
    // Enable blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//}