#ifndef SKYBOX_H
#define SKYBOX_H

#include "loadTexture.h"
#include "noise.h"
#include "Camera.h"


const char* skybox_vshader =
#include "skybox_vshader.glsl"
;
const char* skybox_fshader =
#include "skybox_fshader.glsl"
;

class Skybox {
public:
    std::unique_ptr<Shader> skyboxShader;
    std::unique_ptr<GPUMesh> skyboxMesh;
    GLuint skyboxTexture;

    Skybox() {
        skyboxShader = std::unique_ptr<Shader>(new Shader());
        skyboxShader->verbose = true;
        skyboxShader->add_vshader_from_source(skybox_vshader);
        skyboxShader->add_fshader_from_source(skybox_fshader);
        skyboxShader->link();

        // Load skybox textures
        const std::string skyList[] = {"miramar_ft", "miramar_bk", "miramar_dn", "miramar_up", "miramar_rt", "miramar_lf"};
        const std::string cubemapLayers[] = {"GL_TEXTURE_CUBE_MAP_POSITIVE_X", "GL_TEXTURE_CUBE_MAP_NEGATIVE_X",
                                         "GL_TEXTURE_CUBE_MAP_POSITIVE_Y", "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y",
                                         "GL_TEXTURE_CUBE_MAP_POSITIVE_Z", "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z"};

        glGenTextures(1, &skyboxTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        int tex_wh = 1024;
        for(int i=0; i < 6; ++i) {
            std::vector<unsigned char> image;
            loadTexture(image, (skyList[i]+".png").c_str());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, tex_wh, tex_wh, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        // Generate a cube mesh for skybox
        skyboxMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
        std::vector<Vec3> points;

        points.push_back(Vec3( 1, 1, 1)); // 0
        points.push_back(Vec3(-1, 1, 1)); // 1
        points.push_back(Vec3( 1, 1,-1)); // 2
        points.push_back(Vec3(-1, 1,-1)); // 3
        points.push_back(Vec3( 1,-1, 1)); // 4
        points.push_back(Vec3(-1,-1, 1)); // 5
        points.push_back(Vec3(-1,-1,-1)); // 6
        points.push_back(Vec3( 1,-1,-1)); // 7

        std::vector<unsigned int> indices = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
        skyboxMesh->set_vbo<Vec3>("vposition", points);
        skyboxMesh->set_triangles(indices);
    }

    void draw(Camera camera) {
        skyboxShader->bind();

        // Set transformations
        skyboxShader->set_uniform("V", camera.viewMatrix());
        skyboxShader->set_uniform("P", camera.projectionMatrix());

        // Bind Textures and set uniform
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        skyboxShader->set_uniform("noiseTex", 0);

        // Set atrributes and draw cube using GL_TRIANGLE_STRIP mode
        glEnable(GL_DEPTH_TEST);
        skyboxMesh->set_attributes(*skyboxShader);
        skyboxMesh->set_mode(GL_TRIANGLE_STRIP);
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(resPrim);
        skyboxMesh->draw();

        skyboxShader->unbind();
    }
};


#endif