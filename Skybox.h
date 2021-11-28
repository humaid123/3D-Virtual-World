#ifndef SKYBOX_H
#define SKYBOX_H

#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"
#include <iostream>

#include "loadTexture.h"
//#include "noise.h"
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
	Vec3(-1.0f, -1.0f,  1.0f),//        7--------6
	Vec3(1.0f, -1.0f,  1.0f),//       /|       /|
	Vec3(1.0f, -1.0f, -1.0f),//      4--------5 |
	Vec3(-1.0f, -1.0f, -1.0f),//      | |      | |
	Vec3(-1.0f,  1.0f,  1.0f),//      | 3------|-2
	Vec3(1.0f,  1.0f,  1.0f),//      |/       |/
	Vec3(1.0f,  1.0f, -1.0f),//      0--------1
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

public:
	Skybox() {
		skyboxShader = std::unique_ptr<Shader>(new Shader());
		skyboxShader->verbose = true;
		skyboxShader->add_vshader_from_source(skybox_vshader);
		skyboxShader->add_fshader_from_source(skybox_fshader);
		skyboxShader->link();



		// Load skybox textures
		const std::string skyList[] = { "miramar_ft", "miramar_bk", "miramar_dn", "miramar_up", "miramar_rt", "miramar_lf" };
		// noot rlfb, not lrfb, lrbk, rlbf, 

		// lrdufb is almost good => need to change one up
		//const std::string skyList[] = { "miramar_ft", "miramar_rt", "miramar_bk", "miramar_lf", "miramar_dn", "miramar_up"  };

		
		// auto cubemapLayers = { 
		//	GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
		//	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		//	GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
		//	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		//	GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 
		//	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		//};

		// create skybox texture
		glGenTextures(1, &skyboxTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		// set texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// load images
		int tex_wh = 1024;
		for (int i = 0; i < 6; ++i) {
			std::vector<unsigned char> image;
			loadTexture(image, (skyList[i] + ".png").c_str());
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, tex_wh, tex_wh, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
		}

		std::string name = "cloud.png";
		loadTexture(cloudTexture, name.c_str());
		cloudTexture->bind();
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



		// std::vector<unsigned int> indices = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
		skyboxMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
		skyboxMesh->set_vbo<Vec3>("vposition", skyboxVertices);
		skyboxMesh->set_triangles(skyboxIndices);
		std::cout << "skybox fully loaded\n" << std::endl;
	}

	void draw(Camera camera, float time) {
		// Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
		
		glDepthFunc(GL_LEQUAL);

		// skyboxShader.Activate();
		skyboxShader->bind();


		// glm::mat4 view = glm::mat4(1.0f);
		// glm::mat4 projection = glm::mat4(1.0f);
		// // We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
		// // The last row and column affect the translation of the skybox (which we don't want to affect)
		// view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up)));
		// projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
		// glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		// glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		skyboxShader->set_uniform("view", camera.viewMatrix());
		skyboxShader->set_uniform("projection", camera.projectionMatrix());
		skyboxShader->set_uniform("viewPos", camera.cameraPos);
		skyboxShader->set_uniform("time", time);


		// // Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
		// // where an object is present (a depth of 1.0f will always fail against any object's depth value)
		// glBindVertexArray(skyboxVAO);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		skyboxShader->set_uniform("noiseTex", 0);


		glActiveTexture(GL_TEXTURE1);
		cloudTexture->bind();
		skyboxShader->set_uniform("cloudTexture", 1);

		// glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		// glBindVertexArray(0);

		skyboxMesh->set_attributes(*skyboxShader);
		// unused - skyboxMesh->set_mode(GL_TRIANGLE_STRIP);
		// unused - glEnable(GL_PRIMITIVE_RESTART);
		// unused - glPrimitiveRestartIndex(resPrim);
		skyboxMesh->draw();

		// Switch back to the normal depth function
		glDepthFunc(GL_LESS);

		skyboxShader->unbind();

	}

};

#endif