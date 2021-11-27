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
    float gravityEffect; // 1 => affcted by gravity, 0 => not affected by gravity
    float lifeLength; // how long to stay alive
    // prettify
    float rotation;
    float scale;
    
    float timeSpawned; // time to check when to kill

public:
    Particle(Vec3 _position, Vec3 _velocity, float _gravityEffect, 
    float _lifeLength, float _rotation, float _scale, float _timeSpawned) : 
        position(_position), velocity(_velocity), gravityEffect(_gravityEffect), 
        lifeLength(_lifeLength), rotation(_rotation), scale(_scale), timeSpawned(_timeSpawned) {

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

        // values so that the corners are (x, y) (-x, y), (x, -y), (-x, -y)
        // we translate the water height in the shader itself... so we need a z=0 quad...
        float x = 0.5;
        float y = 0.5;
        points.push_back(Vec3(-x, -y, 0.0f)); texCoords.push_back(Vec2(0, 0));
        points.push_back(Vec3(-x, y, 0.0f)); texCoords.push_back(Vec2(0, 1));
        points.push_back(Vec3(x, -y, 0.0f)); texCoords.push_back(Vec2(1, 0));
        points.push_back(Vec3(x, y, 0.0f)); texCoords.push_back(Vec2(1, 1));

        particleMesh->set_vbo<Vec3>("vposition", points);
        particleMesh->set_triangles(indices);
        particleMesh->set_vtexcoord(texCoords);
    }

    // return if the particle is alive or not and update it => needs to be call every frame
    bool update(float time) {
        velocity.y() += GRAVITY * gravityEffect * time;

        Vec3 change_in_position = (velocity) * time;
        position += change_in_position;
        return (time - timeSpawned) < lifeLength;
    }


    // required to do it here so all particles point towards the camera and not be thin lines
    void updateModelViewMatrix(Camera camera) {
        Mat4x4 modelMatrix = Mat4x4();
        // we need a matrix that only applies the translations so that the view matrix's rotation does not occur
        // to do this, we just need the model matrix to be the transpose of the view matrix => the 4th row and column of the view matrix does not happen
        Mat4x4 viewMatrix = camera.viewMatrix();
        modelMatrix(0, 0) = viewMatrix(0, 0);
	    modelMatrix(0, 1) = viewMatrix(1, 0);
	    modelMatrix(0, 2) = viewMatrix(2, 0);
	    modelMatrix(1, 0) = viewMatrix(0, 1);
	    modelMatrix(1, 1) = viewMatrix(1, 1);
	    modelMatrix(1, 2) = viewMatrix(2, 1);
	    modelMatrix(2, 0) = viewMatrix(0, 2);
	    modelMatrix(2, 1) = viewMatrix(1, 2);
	    modelMatrix(2, 2) = viewMatrix(2, 2);


        // VI ==== NEED TO READ HOW TRANSFORMS WORK in OpenGP's Eigen
        // need to rotate and scale by the rotation and scale float that we are storing
        // modelMatrix = rotate(modelMatrix, Vec3(0, 1, 0), toRadians(rotation));
        // modelMatrix = scale(modelMatrix, Vec3(scale, scale, scale));
        modelViewMatrix = viewMatrix * modelMatrix;
    }

    void draw(Camera camera) {
        // viewMatrix => camera.viewMatrix()
        particleShader->bind();

        updateModelViewMatrix(camera);        
        particleShader->set_uniform("modelViewMatrix", modelViewMatrix);
        particleShader->set_uniform("projectionMatrix", camera.projectionMatrix());
        particleMesh->draw();
        
        particleShader->unbind();
    }

};

#endif