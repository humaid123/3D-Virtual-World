#ifndef SKYBOX_H
#define SKYBOX_H

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"
#include <iostream>

#include "loadTexture.h"
#include "Camera.h"

const char* skybox_vshader =
#include "skybox_vert.glsl"
;

const char* skybox_fshader =
#include "skybox_frag.glsl"
;

std::vector<Vec3> skyboxVertices =
{
	//   Coordinates
	Vec3(-1.0f, -1.0f,  1.0f), //        7--------6
	Vec3(1.0f, -1.0f,  1.0f),  //       /|       /|
	Vec3(1.0f, -1.0f, -1.0f),  //      4--------5 |
	Vec3(-1.0f, -1.0f, -1.0f), //      | |      | |
	Vec3(-1.0f,  1.0f,  1.0f), //      | 3------|-2
	Vec3(1.0f,  1.0f,  1.0f),  //      |/       |/
	Vec3(1.0f,  1.0f, -1.0f),  //      0--------1
	Vec3(-1.0f,  1.0f, -1.0f)
};

std::vector<unsigned int> skyboxIndices =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};



class Skybox {
public:
	std::unique_ptr<Shader> skyboxShader;
	std::unique_ptr<GPUMesh> skyboxMesh;
	GLuint skyboxTexture;

	std::unique_ptr<RGBA8Texture> cloudTexture;
	Vec3 skyColor;

public:
	Skybox(Vec3 _skyColor) {
		skyboxShader = std::unique_ptr<Shader>(new Shader());
		skyboxShader->verbose = true;
		skyboxShader->add_vshader_from_source(skybox_vshader);
		skyboxShader->add_fshader_from_source(skybox_fshader);
		skyboxShader->link();
		skyColor = _skyColor;

		// Load skybox textures
		const std::string skyList[] = { "miramar_ft", "miramar_bk", "miramar_dn", "miramar_up", "miramar_rt", "miramar_lf" };

		// create skybox texture
		glGenTextures(1, &skyboxTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		// set texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// load CUBE_MAP textures => the string name array already have them correctly sorted
		int tex_wh = 1024;
		for (int i = 0; i < 6; ++i) {
			std::vector<unsigned char> image;
			loadTexture(image, (skyList[i] + ".png").c_str());
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, tex_wh, tex_wh, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
		}

		// load cloud texture
		std::string name = "cloud.png";
		loadTexture(cloudTexture, name.c_str());
		cloudTexture->bind();
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		skyboxMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
		skyboxMesh->set_vbo<Vec3>("vposition", skyboxVertices);
		skyboxMesh->set_triangles(skyboxIndices);
	}

	void draw(Camera camera, float time) {
		
		// we use GL_LEQUAL as the depth of the cubemap will always be 1 and we don't want it to get discarded
		glDepthFunc(GL_LEQUAL);
		skyboxShader->bind();

		skyboxShader->set_uniform("view", camera.viewMatrix());
		skyboxShader->set_uniform("projection", camera.projectionMatrix());
		skyboxShader->set_uniform("time", time);
		skyboxShader->set_uniform("baseSkyColor", skyColor);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		skyboxShader->set_uniform("skybox", 0);

		glActiveTexture(GL_TEXTURE1);
		cloudTexture->bind();
		skyboxShader->set_uniform("cloudTexture", 1);

		skyboxMesh->set_attributes(*skyboxShader);
		skyboxMesh->draw();

		// Switch back to the normal depth function
		skyboxShader->unbind();
		glDepthFunc(GL_LESS);
	}

};

#endif