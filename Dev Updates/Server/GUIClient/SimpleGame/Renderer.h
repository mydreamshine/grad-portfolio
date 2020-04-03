#pragma once

#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>
#include <cstdlib>

#include "Dependencies\glew.h"

#include "Dependencies\glm/glm.hpp"
#include "Dependencies\glm/gtc/matrix_transform.hpp"
#include "Dependencies\glm/gtx/euler_angles.hpp"
#include "LoadPng.h"

#define MAX_TEXTURES 1000

using namespace std;

class Renderer
{
public:
	Renderer(int windowSizeX, int windowSizeY);
	~Renderer();

	bool IsInitialized();

	void DrawSolidRect(
		float x, float y, float z, 
		float size, 
		float r, float g, float b, float a);

	void DrawSolidRect(
		float x, float y, float z,
		float sizeX, float sizeY, float sizeZ,
		float r, float g, float b, float a);

	void DrawTextureRect(
		float x, float y, float z,
		float sizeX, float sizeY, float sizeZ,
		float r, float g, float b, float a,
		int textureID);

	int GenPngTexture(char * filePath, GLuint sampling = GL_NEAREST);
	int GenBmpTexture(char * filePath, GLuint sampling = GL_NEAREST);
	bool DeleteTexture(int idx, bool printLog = false);

private:
	void Initialize(int windowSizeX, int windowSizeY);
	bool ReadFile(char* filename, std::string *target);
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShaders(char* filenameVS, char* filenameFS);
	void CreateVertexBufferObjects();

	bool m_Initialized = false;
	
	unsigned int m_WindowSizeX = 0;
	unsigned int m_WindowSizeY = 0;

	//camera position
	glm::vec3 m_v3Camera_Position;
	//camera lookat position
	glm::vec3 m_v3Camera_Lookat;
	//camera up vector
	glm::vec3 m_v3Camera_Up;

	glm::mat4 m_m4OrthoProj;
	glm::mat4 m_m4PersProj;
	glm::mat4 m_m4Model;
	glm::mat4 m_m4View;
	glm::mat4 m_m4ProjView;

	//GL resources :: must destory before exit
	GLuint m_VBORect = 0;
	GLuint m_SolidRectShader = 0;
	GLuint m_TextureRectShader = 0;
	int m_Textures[MAX_TEXTURES];
};

