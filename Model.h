#pragma once
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <vector>
#include <string>
#include <cmath>
#include "maths_funcs.h"

#include <GL/glew.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations


#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

class Model
{
public:
	Model(std::string filename, GLuint shaderID);
	ModelData model_data;
	GLuint loc1, loc2, loc3;
	std::vector<int> vertex_nums;
	GLuint shaderProgramID;
	GLuint vao;
	std::vector<GLuint> vp_vbo;
	std::vector<GLuint> vn_vbo;
	std::vector<GLuint> vt_vbo;

private:
	ModelData load_mesh(const char* file_name);
	void generateObjectBufferMesh();
};