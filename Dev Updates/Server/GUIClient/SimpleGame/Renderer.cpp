#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	//default settings
	glClearDepth(1.f);

	Initialize(windowSizeX, windowSizeY);
}


Renderer::~Renderer()
{
	//delete all resources here
	glDeleteShader(m_SolidRectShader);
	glDeleteShader(m_TextureRectShader);
	glDeleteBuffers(1, &m_VBORect);
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		DeleteTexture(i);
	}
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	//Initialize Texture arrays
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		m_Textures[i] = -1;;
	}

	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_TextureRectShader = CompileShaders("./Shaders/TextureRect.vs", "./Shaders/TextureRect.fs");
	
	//Create VBOs
	CreateVertexBufferObjects();

	//Initialize camera settings
	m_v3Camera_Position = glm::vec3(0.f, 0.f, 1000.f);
	m_v3Camera_Lookat = glm::vec3(0.f, 0.f, 0.f);
	m_v3Camera_Up = glm::vec3(0.f, 1.f, 0.f);
	m_m4View = glm::lookAt(
		m_v3Camera_Position,
		m_v3Camera_Lookat,
		m_v3Camera_Up
	);

	//Initialize projection matrix
	m_m4OrthoProj = glm::ortho(
		-(float)windowSizeX/2.f, (float)windowSizeX/2.f, 
		-(float)windowSizeY/2.f, (float)windowSizeY/2.f,
		0.0001f, 10000.f);
	m_m4PersProj = glm::perspectiveRH(45.f, 1.f, 1.f, 1000.f);

	//Initialize projection-view matrix
	m_m4ProjView = m_m4OrthoProj * m_m4View; //use ortho at this time
	//m_m4ProjView = m_m4PersProj * m_m4View;

	//Initialize model transform matrix :; used for rotating quad normal to parallel to camera direction
	m_m4Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(1.f, 0.f, 0.f));

	m_Initialized = true;
}

bool Renderer::IsInitialized()
{
	return m_Initialized;
}

void Renderer::CreateVertexBufferObjects()
{
	float rectSize = 0.5f;
	float rect[]
		=
	{
		-rectSize, -rectSize, 0.f, -rectSize, rectSize, 0.f, rectSize, rectSize, 0.f, //Triangle1
		-rectSize, -rectSize, 0.f,  rectSize, rectSize, 0.f, rectSize, -rectSize, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
}

int Renderer::GenPngTexture(char * filePath, GLuint sampling)
{
	//Load Pngs
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filePath);
	if (error != 0)
	{
		cout << "PNG Image Loading Failed : " << filePath << endl;
		assert(0);
	}

	GLuint temp;
	glGenTextures(1, &temp);

	if (temp < 0)
	{
		cout << "PNG Texture Creation Failed : " << filePath << endl;
		assert(0);
	}

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampling);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	return temp;
}

int Renderer::GenBmpTexture(char * filePath, GLuint sampling)
{
	unsigned int width, height;

	const unsigned char* rawImage = loadBMP::loadBMPRaw(filePath, width, height, false);

	if (rawImage == NULL)
	{
		cout << "BMP Image Loading Failed : " << filePath << endl;
		assert(0);
	}

	GLuint temp;
	glGenTextures(1, &temp);

	if (temp < 0)
	{
		cout << "BMP Texture Creation Failed : " << filePath << endl;
		assert(0);
	}

	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampling);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &rawImage[0]);

	return temp;
}

bool Renderer::DeleteTexture(int idx, bool printLog)
{
	if (idx <= 0)
	{
		if (printLog)
		{
			cout << "Error : Texture index is negative " << idx << endl;
		}
		return false;
	}
	if (m_Textures[idx] == -1)
	{
		if (printLog)
		{
			cout << "Error : Texture " << idx << " already deleted. " << endl;
		}
		return false;
	}
	glDeleteTextures(1, (const GLuint*)(&m_Textures[idx]));
	m_Textures[idx] = -1;

	return true;
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//쉐이더 오브젝트 생성
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	//쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	//할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj 가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram 에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string *target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //빈 쉐이더 프로그램 생성

	if (ShaderProgram == 0) { //쉐이더 프로그램이 만들어졌는지 확인
		fprintf(stderr, "Error creating shader program\n");
		assert(0);
	}

	std::string vs, fs;

	//shader.vs 가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		assert(0);
		return -1;
	};

	//shader.fs 가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		assert(0);
		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach 완료된 shaderProgram 을 링킹함
	glLinkProgram(ShaderProgram);

	//링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		assert(0);
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		assert(0);
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.\n";

	return ShaderProgram;
}

void Renderer::DrawSolidRect(float x, float y, float z, float size, float r, float g, float b, float a)
{
	//Program select
	GLuint shader = m_SolidRectShader;

	glUseProgram(shader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	GLuint uProjView = glGetUniformLocation(shader, "u_ProjView");
	GLuint uRotToCam = glGetUniformLocation(shader, "u_RotToCam");
	GLuint uTrans = glGetUniformLocation(shader, "u_Trans");
	GLuint uScale = glGetUniformLocation(shader, "u_Scale");
	GLuint uColor = glGetUniformLocation(shader, "u_Color");

	glUniform3f(uTrans, x, y, z);
	glUniform3f(uScale, size, size, size);
	glUniform4f(uColor, r, g, b, a);
	glUniformMatrix4fv(uProjView, 1, GL_FALSE, &m_m4ProjView[0][0]);
	glUniformMatrix4fv(uRotToCam, 1, GL_FALSE, &m_m4Model[0][0]);

	int attribPosition = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_BLEND);
}

void Renderer::DrawSolidRect(
	float x, float y, float z, 
	float sizeX, float sizeY, float sizeZ,
	float r, float g, float b, float a)
{
	//Program select
	GLuint shader = m_SolidRectShader;

	glUseProgram(shader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	GLuint uProjView = glGetUniformLocation(shader, "u_ProjView");
	GLuint uRotToCam = glGetUniformLocation(shader, "u_RotToCam");
	GLuint uTrans = glGetUniformLocation(shader, "u_Trans");
	GLuint uScale = glGetUniformLocation(shader, "u_Scale");
	GLuint uColor = glGetUniformLocation(shader, "u_Color");

	glUniform3f(uTrans, x, y, z);
	glUniform3f(uScale, sizeX, sizeY, sizeZ);
	glUniform4f(uColor, r, g, b, a);
	glUniformMatrix4fv(uProjView, 1, GL_FALSE, &m_m4ProjView[0][0]);
	glUniformMatrix4fv(uRotToCam, 1, GL_FALSE, &m_m4Model[0][0]);

	int attribPosition = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_BLEND);
}

void Renderer::DrawTextureRect(
	float x, float y, float z,
	float sizeX, float sizeY, float sizeZ,
	float r, float g, float b, float a,
	int textureID)
{
	//Program select
	GLuint shader = m_TextureRectShader;

	glUseProgram(shader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	GLuint uProjView = glGetUniformLocation(shader, "u_ProjView");
	GLuint uRotToCam = glGetUniformLocation(shader, "u_RotToCam");
	GLuint uTrans = glGetUniformLocation(shader, "u_Trans");
	GLuint uScale = glGetUniformLocation(shader, "u_Scale");
	GLuint uColor = glGetUniformLocation(shader, "u_Color");
	GLuint uTexture = glGetUniformLocation(shader, "u_Texture");

	glUniformMatrix4fv(uProjView, 1, GL_FALSE, &m_m4ProjView[0][0]);
	glUniformMatrix4fv(uRotToCam, 1, GL_FALSE, &m_m4Model[0][0]);
	glUniform3f(uTrans, x, y, z);
	glUniform3f(uScale, sizeX, sizeY, sizeZ);
	glUniform4f(uColor, r, g, b, a);
	glUniform1i(uTexture, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int attribPosition = glGetAttribLocation(shader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}