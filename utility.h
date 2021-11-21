#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename) {
	std::ifstream in(filename, std::ios::binary);
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

typedef float Scalar;
typedef Eigen::Transform<float, 3, Eigen::Affine> Transform;
typedef Eigen::Matrix<Scalar, 2, 1> Vec2;
typedef Eigen::Matrix<Scalar, 3, 1> Vec3;
typedef Eigen::Matrix<Scalar, 4, 4> Mat4x4;


#endif