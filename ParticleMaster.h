
#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

// if this is working => you need to add a Particle Generator => see the zip files....
#include <iostream>
#include "loadTexture.h"

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"

#include "Camera.h"
#include "Particle.h"


class ParticleSystem {
public:
    std::vector<Particle> particles;

    ParticleSystem() {

    }

    void update(float time) {
        for (auto it = particles.begin(); it != particles.end(); it++) {
            bool stillAlive = it->update(time);
            if (!stillAlive) {
                particles.erase(it);
            }
        }
    }

    void draw(Camera camera) {
        // a few opengl calls to prettify
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // disable writing to the depth buffer

        for (auto it = particles.begin(); it != particles.end(); it++) {
            it->draw(camera);
        }
            // clean up
        glDepthMask(GL_TRUE); // disable writing to the depth buffer
        glDisable(GL_BLEND);
    }

    // void addParticle(Particle particle) {
    //    particles.push_back(particle);
    // }
};


#endif