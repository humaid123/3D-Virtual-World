#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"

#include "loadTexture.h"
// #include "noise.h"
#include "Terrain.h"
#include "Skybox.h"
#include "Camera.h"
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
    Camera camera(width, height);


    // Display callback
    Window& window = app.create_window([&](Window&){
        glViewport(0,0,width,height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        terrain.draw(camera);
        skybox.draw(camera);
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