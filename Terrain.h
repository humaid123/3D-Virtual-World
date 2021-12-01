#ifndef TERRAIN_H
#define TERRAIN_H

#include <iostream>
#include "loadTexture.h"

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"
#include <climits>

#include "Camera.h"

const char* terrain_vshader =
#include "terrain_vshader.glsl"
;

const char* terrain_fshader =
#include "terrain_fshader.glsl"
;


const unsigned indexRestart = UINT_MAX;

class Terrain {
public:
    std::unique_ptr<Shader> terrainShader;
    std::unique_ptr<GPUMesh> terrainMesh;
    std::map<std::string, std::unique_ptr<RGBA8Texture>> terrainTextures;
    float waterHeight;
    Vec3 skyColor, lightPos;

    Terrain(float size_grid_x, float size_grid_y, float _waterHeight, Vec3 _skyColor, Vec3 _lightPos) 
        : waterHeight(_waterHeight), skyColor(_skyColor), lightPos(_lightPos) {
        terrainShader = std::unique_ptr<Shader>(new Shader());
        terrainShader->verbose = true;
        terrainShader->add_vshader_from_source(terrain_vshader);
        terrainShader->add_fshader_from_source(terrain_fshader);
        terrainShader->link();

        const std::string list[] = { "grass", "rock", "sand", "snow" };
        for (int i=0 ; i < 4 ; ++i) {
            loadTexture(terrainTextures[list[i]], (list[i]+".png").c_str());
            terrainTextures[list[i]]->bind();
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        terrainMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
        // divides the plane rectangle into n_width along the width and n_height along the height to create a 
        // displaceable grid.
        int n_width =  1024;
        int n_height = 1024;

        std::vector<Vec3> points;
        std::vector<unsigned int> indices;

        for (int j = 0; j < n_width; ++j) {
            for (int i = 0; i < n_height; ++i) {
                // we create a vertices from [-size_grid_x, size_grid_x] instead of [-1, 1] => this creates a larger terrain 
                // that we can blur to give the illusion that it is infinite
                float vertX = -size_grid_x / 2 + j / (float)n_width * size_grid_x;
                float vertY = -size_grid_y / 2 + i / (float)n_height * size_grid_y;
                float vertZ = 0.0f;
                points.push_back(Vec3(vertX, vertY, vertZ));
            }
        }

        // we need to build the grid with GL_TRIANGLE_STRIP
        // the best way is to build each row into a strip
        // then concatenate the strips
        // opengl provides  glEnable(GL_PRIMITIVE_RESTART) and  glPrimitiveRestartIndex(index)
        // such that when the given index is reached, the next primitive is built
        for(int j = 0; j < n_width - 1; ++j) {

            // Push two vertices at the base of each strip
            float baseX = j * n_width;
            indices.push_back(baseX);
            float baseY = ((j + 1) * n_width);
            indices.push_back(baseY);

            for(int i = 1; i < n_height; ++i) {
            
                // Calculate next two vertices
                float tempX = i + j * n_width;
                indices.push_back(tempX);
                float tempY = i + (j + 1) * n_height;
                indices.push_back(tempY);
            }

            // A new strip will begin when this index is reached
            indices.push_back(indexRestart);
        }

        terrainMesh->set_vbo<Vec3>("vposition", points);
        terrainMesh->set_triangles(indices);
    }

    void draw(Camera camera, Vec3 clipPlaneNormal, float clipPlaneHeight) {
        terrainShader->bind();

        // Generate and set Model
        Mat4x4 M = Mat4x4::Identity();
        terrainShader->set_uniform("M", M);
        terrainShader->set_uniform("V", camera.viewMatrix());
        terrainShader->set_uniform("P", camera.projectionMatrix());
        terrainShader->set_uniform("viewPos", camera.cameraPos);

        // set clipping plane for the reflection and refraction
        terrainShader->set_uniform("clipPlaneNormal", clipPlaneNormal);
        terrainShader->set_uniform("clipPlaneHeight", clipPlaneHeight);
        terrainShader->set_uniform("waterHeight", waterHeight);
        terrainShader->set_uniform("skyColor", skyColor);
        terrainShader->set_uniform("lightPos", lightPos);

        int i = 0;
        for (auto it = terrainTextures.begin(); it != terrainTextures.end(); ++it) {
            glActiveTexture(GL_TEXTURE1 + i);
            (it->second)->bind();
            terrainShader->set_uniform(it->first.c_str(), 1 + i);
            ++i;
        }

        // Draw terrain using triangle strips
        terrainMesh->set_attributes(*terrainShader);
        terrainMesh->set_mode(GL_TRIANGLE_STRIP);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(indexRestart);
        terrainMesh->draw();

        terrainShader->unbind();
    }
};

#endif