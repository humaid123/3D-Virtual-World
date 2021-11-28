#ifndef PARTICLE_H
#define PARTICLE_H

#include <iostream>
#include "loadTexture.h"
//#include "noise.h"

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"

#include "Camera.h"

const char* particle_vshader =
#include "particle_vshader.glsl"
;

const char* particle_fshader =
#include "particle_fshader.glsl"
;

const float GRAVITY = -50;

class Particle {
public:
    std::unique_ptr<Shader> particleShader;
    std::unique_ptr<GPUMesh> particleMesh;

    Mat4x4 modelViewMatrix;
    // Mat4X4 projectionMatrix;

    Vec3 position;
    Vec3 velocity;
    float gravityEffect; // 1 => affected by gravity, 0 => not affected by gravity
    float lifeLength; // how long to stay alive
    // prettify
    float rotation;
    float scaling;
    
    float timeSpawned; // time to check when to kill

public:
    Particle(Vec3 _position, Vec3 _velocity, float _gravityEffect, 
        float _lifeLength, float _rotation, float _scale, 
        float _timeSpawned) : position(_position), velocity(_velocity), 
            gravityEffect(_gravityEffect), lifeLength(_lifeLength), rotation(_rotation), scaling(_scale), timeSpawned(_timeSpawned) {

        std::cout << gravityEffect << " " << lifeLength << " " << rotation << " " << scaling << " " << timeSpawned << std::endl;
        particleShader = std::unique_ptr<Shader>(new Shader());
        particleShader->verbose = true;
        particleShader->add_vshader_from_source(particle_vshader);
        particleShader->add_fshader_from_source(particle_fshader);
        particleShader->link();



//        particleShader->set_attributes("position", position);

    // simple quad mesh
        particleMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
        std::vector<Vec3> points;
        std::vector<unsigned int> indices = {0, 2, 1, 2, 3, 1};
        std::vector<Vec2> texCoords;

        // // values so that the corners are (x, y) (-x, y), (x, -y), (-x, -y)
        // // we translate the water height in the shader itself... so we need a z=0 quad...
        float x = 0.5;
        float z = 0.5;
        // // points.push_back(Vec3(0.0f, -x, -z)); texCoords.push_back(Vec2(0, 0));
        // // points.push_back(Vec3(0.0f, -x, z)); texCoords.push_back(Vec2(0, 1));
        //  // points.push_back(Vec3(0.0f, x, -z)); texCoords.push_back(Vec2(1, 0));
        // // points.push_back(Vec3(0.0f, x, z)); texCoords.push_back(Vec2(1, 1));
        points.push_back(Vec3(-x, -z, 0.0f)); texCoords.push_back(Vec2(0, 0));
        points.push_back(Vec3(-x, z, 0.0f)); texCoords.push_back(Vec2(0, 1));
        points.push_back(Vec3(x, -z, 0.0f)); texCoords.push_back(Vec2(1, 0));
        points.push_back(Vec3(x, z, 0.0f)); texCoords.push_back(Vec2(1, 1));
       

        particleMesh->set_vbo<Vec3>("vposition", points);
        particleMesh->set_triangles(indices);
        particleMesh->set_vtexcoord(texCoords);
    }

    // return if the particle is alive or not and update it => needs to be call every frame
    bool update(float time, float fps) {
        velocity.z() -= GRAVITY/1000 * gravityEffect * 1 / fps;
        Vec3 change_in_position = (velocity) * 1 / fps;
        position += change_in_position;
        return (time - timeSpawned) < lifeLength;
    }

    void draw(Camera camera) {
        particleShader->bind();
             
        // particleShader->set_uniform("VM", camera.viewMatrix());
        particleShader->set_uniform("P", camera.projectionMatrix());
        particleShader->set_uniform("V", camera.viewMatrix());
        particleShader->set_uniform("viewPos", camera.cameraPos);


        particleMesh->set_attributes(*particleShader);
        particleMesh->draw();
        
        particleShader->unbind();
    }

};

#endif