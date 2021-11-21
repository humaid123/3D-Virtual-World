#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

class Shader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;

	// Constructor that build the Shader Program from 2 different shaders
	Shader(const char* vertexFile, const char* fragmentFile) {
		// Read vertexFile and fragmentFile and store the strings
		std::string vertexCode = get_file_contents(vertexFile);
		std::string fragmentCode = get_file_contents(fragmentFile);

		// Convert the shader source strings into character arrays
		const char* vertexSource = vertexCode.c_str();
		const char* fragmentSource = fragmentCode.c_str();

		// Create Vertex Shader Object and get its reference
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		// Attach Vertex Shader source to the Vertex Shader Object
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		// Compile the Vertex Shader into machine code
		glCompileShader(vertexShader);
		// Checks if Shader compiled succesfully
		compileErrors(vertexShader, "VERTEX");

		// Create Fragment Shader Object and get its reference
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		// Attach Fragment Shader source to the Fragment Shader Object
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		// Compile the Vertex Shader into machine code
		glCompileShader(fragmentShader);
		// Checks if Shader compiled succesfully
		compileErrors(fragmentShader, "FRAGMENT");

		// Create Shader Program Object and get its reference
		ID = glCreateProgram();
		// Attach the Vertex and Fragment Shaders to the Shader Program
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		// Wrap-up/Link all the shaders together into the Shader Program
		glLinkProgram(ID);
		// Checks if Shaders linked succesfully
		compileErrors(ID, "PROGRAM");

		// Delete the now useless Vertex and Fragment Shader objects
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	// Activates the Shader Program
	void activate() {
		glUseProgram(ID);
	}

	// VI --- VI look to add destructor ????
	// Deletes the Shader Program
	void Delete() {
		glDeleteProgram(ID);
	}


	// utility uniform functions
void setBool(const std::string &name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void setInt(const std::string &name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void setFloat(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void setVec2(const std::string &name, Vec2 vector) const {
	unsigned int location = glGetUniformLocation(ID, name.c_str());
	glUniform2fv(location, 1, vector.data()));
}

void setVec3(const std::string &name, Vec3 vector) const {
	unsigned int location = glGetUniformLocation(ID, name.c_str());
	glUniform3fv(location, 1, vector.data());
}

void setVec4(const std::string &name, Vec4 vector) const {
	unsigned int location = glGetUniformLocation(ID, name.c_str());
	glUniform4fv(location, 1, vector.data());
}

void setMat4(const std::string &name, Mat4x4 matrix) const {
	unsigned int mat = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(mat, 1, false, matrix.data());
}

void setSampler2D(const std::string &name, unsigned int texture, int id) const {
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->setInt(name, id);
}
void setSampler3D(const std::string &name, unsigned int texture, int id) const {
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_3D, texture);
	this->setInt(name, id);
}

private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type) {
		// Stores status of compilation
		GLint hasCompiled;
		// Character array to store error message in
		char infoLog[1024];
	
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
			if (hasCompiled == GL_FALSE) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
			}
		} else {
			glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
			if (hasCompiled == GL_FALSE) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
			}
		}

	}
};


#endif