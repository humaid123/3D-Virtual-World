#ifndef WATER_H
#define WATER_H

#include "utility.h"
#include "Camera.h"

const char* water_vshader =
#include "water_vshader.glsl"
;

const char* water_fshader =
#include "water_fshader.glsl"
;

class Water {
public:
    std::unique_ptr<Shader> waterShader;
    std::unique_ptr<GPUMesh> waterMesh;

    Mat4x4 M = Mat4x4::Identity(); // the model is an identity => we displace manually

    std::unique_ptr<Framebuffer> reflectionFBO;
    std::unique_ptr<Framebuffer> refractionFBO;
    std::unique_ptr<RGBA8Texture> reflectionTexture;
    std::unique_ptr<RGBA8Texture> refractionTexture;
    std::unique_ptr<D16Texture> reflectionDepthTexture;
    std::unique_ptr<D16Texture> refractionDepthTexture;

    // make the FBO slightly smaller so that the rendering is faster
    int reflectionWidth = 640, reflectionHeight = 360;
    int refractionWidth = 320, refractionHeight = 180;

    // water.png texture
    std::unique_ptr<RGBA8Texture> waterTexture;

public:
    Water(float size_grid_x, float size_grid_y, float waterHeight) {
        waterShader = std::unique_ptr<Shader>(new Shader());
        waterShader->verbose = true;
        waterShader->add_vshader_from_source(water_vshader);
        waterShader->add_fshader_from_source(water_fshader);
        waterShader->link();

        waterMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
        std::vector<Vec3> points;
        std::vector<unsigned int> indices = {0, 2, 1, 2, 3, 1};
        std::vector<Vec2> texCoords;

        // values so that the corners are (x, y) (-x, y), (x, -y), (-x, -y)
        // we translate the water height in the shader itself... so we need a z=0 quad...
        // we dont need the water quad to cover the whole space, we can just cover 3/4 of it...
        float x = (3.0/4.0 * size_grid_x) / 2.0;
        float y = (3.0/4.0 * size_grid_y) / 2.0;
        points.push_back(Vec3(-x, -y, waterHeight)); texCoords.push_back(Vec2(0, 0));
        points.push_back(Vec3(-x,  y, waterHeight)); texCoords.push_back(Vec2(0, 1));
        points.push_back(Vec3( x, -y, waterHeight)); texCoords.push_back(Vec2(1, 0));
        points.push_back(Vec3( x,  y, waterHeight)); texCoords.push_back(Vec2(1, 1));

        waterMesh->set_vbo<Vec3>("vposition", points);
        waterMesh->set_triangles(indices);
        waterMesh->set_vtexcoord(texCoords);

        // create and load up textures to reflection FBO 
        reflectionFBO = std::unique_ptr<Framebuffer>(new Framebuffer());
        // create and attach the reflection Texture
        reflectionTexture = std::unique_ptr<RGBA8Texture>((new RGBA8Texture()));
        reflectionTexture->allocate(reflectionWidth, reflectionHeight);
        // set up how to interpolate the textures
        glActiveTexture(GL_TEXTURE0);
        reflectionTexture->bind();
        // required as no mipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        reflectionFBO->attach_color_texture(*reflectionTexture);
        // create and attach the depth texture
        reflectionDepthTexture = std::unique_ptr<D16Texture>((new D16Texture()));
        reflectionDepthTexture->allocate(reflectionWidth, reflectionHeight);
        reflectionFBO->attach_depth_texture(*reflectionDepthTexture);

        // create and loadup textures to refraction FBO
        refractionFBO = std::unique_ptr<Framebuffer>(new Framebuffer());
        // create and attach the refraction Texture
        refractionTexture = std::unique_ptr<RGBA8Texture>((new RGBA8Texture()));
        refractionTexture->allocate(refractionWidth, refractionHeight);
        glActiveTexture(GL_TEXTURE1);
        refractionTexture->bind();
        // required as no mipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        refractionFBO->attach_color_texture(*refractionTexture);
        // create and attach the depth texture
        refractionDepthTexture = std::unique_ptr<D16Texture>((new D16Texture()));
        refractionDepthTexture->allocate(refractionWidth, refractionHeight);
        refractionFBO->attach_depth_texture(*refractionDepthTexture);


        // load water texture from water.png
        std::string name = "water.png";
        loadTexture(waterTexture, name.c_str());
        waterTexture->bind();
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }


    void draw(Camera camera, float time) {
        waterShader->bind();

        waterShader->set_uniform("M", M);
        waterShader->set_uniform("V", camera.viewMatrix());
        waterShader->set_uniform("P", camera.projectionMatrix());
        waterShader->set_uniform("viewPos", camera.cameraPos);
        waterShader->set_uniform("time", time);

        glActiveTexture(GL_TEXTURE0);
        reflectionTexture->bind();
        waterShader->set_uniform("reflectionTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        refractionTexture->bind();
        waterShader->set_uniform("refractionTexture", 1);

        glActiveTexture(GL_TEXTURE2);
        waterTexture->bind();
        waterShader->set_uniform("waterTexture", 2);

        waterMesh->set_attributes(*waterShader);
        waterMesh->draw();

        waterShader->unbind();
    }

};



#endif