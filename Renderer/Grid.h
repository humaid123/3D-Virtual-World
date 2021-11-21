#ifndef GRID_H
#define GRID_H

#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"


struct Vertex {
	std:vector<Vec3> position:
	std:vector<Vec2> texCoord;
}

class Grid {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	VAO VAO;


	Grid(width, height, n_width, n_height, z) {
		// look at how to set up this again....
		for (int i = 0; i < n_width; i++) {
			for (int j = 0; j < n_height; j++) {
				Vertex vertex;
				vertex.position = Vec3();
				vertex.texCoords = Vec2();
				vertices.push_back(vertex);
			}
		}
	
		for (int i = 0; i < n_width; i++) {
			for (int j = 0; j < n_height; j++) {
				indices.push_back();
				indices.push_back();
				indices.push_back();

				indices.push_back();
				indices.push_back();
				indices.push_back();
			}
		}

		VAO.Bind();
		// Generates Vertex Buffer Object and links it to vertices
		VBO VBO(vertices);
		// Generates Element Buffer Object and links it to indices
		EBO EBO(indices);

		// Links VBO attributes such as coordinates and colors to VAO
		VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
		VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
		VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
		VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
		// Unbind all to prevent accidentally modifying them
		VAO.Unbind();
		VBO.Unbind();
		EBO.Unbind();
	}

	// the shader will have the noise production inside...
	void draw() {
		VAO.bind();
		// Draw the actual mesh
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		VAO.unbind();

	}
 }

#endif