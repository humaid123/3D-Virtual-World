#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

class Terrain {
public:
	Grid grid;
	Shader shader;
	std::vector<ImageTexture> textures;


	Terrain(hybridFbmVertShader, blinnPhongFragShader, width, height, n_width, n_height, imageTextureNames) {
		shader = Shader(hybridFbmVertShader, blinnPhongFragShader);
		grid = Grid(width, height, n_width, n_height, 0);
		for (std::string name:textureNames) {
			textures.push_back(ImageTexture(name));
		}
	
	}

	draw(Camera cam) {
		shader.activate();
		for (int i = 0; i < textures.size(); i++) {
			shader.setSampler2D(textures[i].name, textures[i].id, i);
		}

		// Take care of the camera Matrix
		shader.setVec3(camera.Position.x, camera.Position.y, camera.Position.z); // required for view direction => WE CAN ALSO USE THIS TO TRANSLATE FOR NOISE!!!
		camera.Matrix(shader, "camMatrix"); // sends P * V as camMatrix uniform

		// we may not have to use this => we can just translate based on the camera position itself....
		// Initialize matrices
		// glm::mat4 trans = glm::mat4(1.0f);
		// Transform the matrices to their correct form
		// trans = glm::translate(trans, translation);
		// Push the matrices to the vertex shader
		// glUniformMatrix4fv(glGetUniformLocation(shader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(trans));
		// glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));
		
		grid.draw(); // grid is displaced by the vertex shader itself
	}

}



#endif