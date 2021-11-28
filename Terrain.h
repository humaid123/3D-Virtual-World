#ifndef TERRAIN_H
#define TERRAIN_H

#include <iostream>
#include "loadTexture.h"

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"

#include "Camera.h"

const char* terrain_vshader =
#include "terrain_vshader.glsl"
;

const char* terrain_fshader =
#include "terrain_fshader.glsl"
;

const unsigned resPrim = 999999;

class Terrain {
public:
    std::unique_ptr<Shader> terrainShader;
    std::unique_ptr<GPUMesh> terrainMesh;
    std::map<std::string, std::unique_ptr<RGBA8Texture>> terrainTextures;
    float waterHeight;
    Vec3 skyColor;
    Vec3 lightPos;

    Terrain(float size_grid_x, float size_grid_y, float _waterHeight, Vec3 _skyColor, Vec3 _lightPos) 
        : waterHeight(_waterHeight), skyColor(_skyColor), lightPos(_lightPos) {
        terrainShader = std::unique_ptr<Shader>(new Shader());
        terrainShader->verbose = true;
        terrainShader->add_vshader_from_source(terrain_vshader);
        terrainShader->add_fshader_from_source(terrain_fshader);
        terrainShader->link();

        const std::string list[] = { "grass", "rock", "sand", "snow", "lunar" };
        for (int i=0 ; i < 5 ; ++i) {
            loadTexture(terrainTextures[list[i]], (list[i]+".png").c_str());
            terrainTextures[list[i]]->bind();
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }


        // Generate a flat mesh for the terrain with given dimensions, using triangle strips
        terrainMesh = std::unique_ptr<GPUMesh>(new GPUMesh());

        // Grid resolution
        int n_width = 1024;
        int n_height = 1024;

        std::vector<Vec3> points;
        std::vector<unsigned int> indices;
        std::vector<Vec2> texCoords;

        // Generate vertex and texture coordinates
        for (int j = 0; j < n_width; ++j) {
            for (int i = 0; i < n_height; ++i) {

                // Calculate vertex positions
                float vertX = -size_grid_x / 2 + j / (float)n_width * size_grid_x;
                float vertY = -size_grid_y / 2 + i / (float)n_height * size_grid_y;
                float vertZ = 0.0f;
                //std::cout << vertX << " " << vertY << std::endl;
                points.push_back(Vec3(vertX, vertY, vertZ));

                // Calculate texture coordinates
                float texX = i / (float)(n_width - 1);
                float texY = j / (float)(n_height - 1);
                texCoords.push_back(Vec2(texX, texY));
            }
        }

        // Generate element indices via triangle strips
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
            indices.push_back(resPrim);
        }

        terrainMesh->set_vbo<Vec3>("vposition", points);
        terrainMesh->set_triangles(indices);
        terrainMesh->set_vtexcoord(texCoords);
    }

    void draw(Camera camera, Vec3 clipPlaneNormal, float clipPlaneHeight) {
        terrainShader->bind();

        // Generate and set Model
        Mat4x4 M = Mat4x4::Identity();
        terrainShader->set_uniform("M", M);

        // Generate and set View
        terrainShader->set_uniform("V", camera.viewMatrix());

        // Generate and set Projection
        terrainShader->set_uniform("P", camera.projectionMatrix());

        // Set camera position
        terrainShader->set_uniform("viewPos", camera.cameraPos);

        // set clipping plane for the reflection and refraction
        terrainShader->set_uniform("clipPlaneNormal", clipPlaneNormal);
        terrainShader->set_uniform("clipPlaneHeight", clipPlaneHeight);
        terrainShader->set_uniform("waterHeight", waterHeight);
        terrainShader->set_uniform("skyColor", skyColor);
        terrainShader->set_uniform("lightPos", lightPos);
        // Bind textures
        int i = 0;
        for (std::map<std::string, std::unique_ptr<RGBA8Texture>>::iterator it = terrainTextures.begin(); it != terrainTextures.end(); ++it) {
            glActiveTexture(GL_TEXTURE1 + i);
            (it->second)->bind();
            terrainShader->set_uniform(it->first.c_str(), 1 + i);
            ++i;
        }

        // Bind height texture and set uniform noiseTex
        glActiveTexture(GL_TEXTURE0);
        //heightTexture->bind();
        //terrainShader->set_uniform("noiseTex", 0);

        // Draw terrain using triangle strips
        //wwwwwwwwwglEnable(GL_DEPTH_TEST);
        terrainMesh->set_attributes(*terrainShader);
        terrainMesh->set_mode(GL_TRIANGLE_STRIP);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(resPrim);

        terrainMesh->draw();

        terrainShader->unbind();
    }
};

#endif