#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_CLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <stdio.h>
#include <string.h>
#include <string>
#include <queue>
#include <float.h>

using namespace glm;
using namespace std;


// -------------------------------------------------
// Global Variables
// -------------------------------------------------
#define M_PI 3.1415926535f
#define INF 1'000'000'000
int Width = 1280;
int Height = 1280;

float  					gTotalTimeElapsed = 0;
int 					gTotalFrames = 0;
GLuint 					gTimer;

std::vector<vec3>	gPositions;
std::vector<vec3>	gNormals;
std::vector<vec3>	gTriangles;
// -------------------------------------------------

struct Triangle
{
	unsigned int 	indices[3];
};


void init_timer()
{
	glGenQueries(1, &gTimer);
}

void start_timing()
{
	glBeginQuery(GL_TIME_ELAPSED, gTimer);
}

float stop_timing()
{
	glEndQuery(GL_TIME_ELAPSED);

	GLint available = GL_FALSE;
	while (available == GL_FALSE)
		glGetQueryObjectiv(gTimer, GL_QUERY_RESULT_AVAILABLE, &available);

	GLint result;
	glGetQueryObjectiv(gTimer, GL_QUERY_RESULT, &result);

	float timeElapsed = result / (1000.0f * 1000.0f * 1000.0f);
	return timeElapsed;
}

void tokenize(char* string, std::vector<std::string>& tokens, const char* delimiter)
{
	char* token = strtok(string, delimiter);
	while (token != NULL)
	{
		tokens.push_back(std::string(token));
		token = strtok(NULL, delimiter);
	}
}

int face_index(const char* string)
{
	int length = strlen(string);
	char* copy = new char[length + 1];
	memset(copy, 0, length + 1);
	strcpy(copy, string);

	std::vector<std::string> tokens;
	tokenize(copy, tokens, "/");
	delete[] copy;
	if (tokens.front().length() > 0 && tokens.back().length() > 0 && atoi(tokens.front().c_str()) == atoi(tokens.back().c_str()))
	{
		return atoi(tokens.front().c_str());
	}
	else
	{
		printf("ERROR: Bad face specifier!\n");
		exit(0);
	}
}

void load_mesh(std::string fileName)
{
	std::ifstream fin(fileName.c_str());
	if (!fin.is_open())
	{
		printf("ERROR: Unable to load mesh from %s!\n", fileName.c_str());
		exit(0);
	}

	float xmin = FLT_MAX;
	float xmax = -FLT_MAX;
	float ymin = FLT_MAX;
	float ymax = -FLT_MAX;
	float zmin = FLT_MAX;
	float zmax = -FLT_MAX;

	while (true)
	{
		char line[1024] = { 0 };
		fin.getline(line, 1024);

		if (fin.eof())
			break;

		if (strlen(line) <= 1)
			continue;

		std::vector<std::string> tokens;
		tokenize(line, tokens, " ");

		if (tokens[0] == "v")
		{
			float x = atof(tokens[1].c_str());
			float y = atof(tokens[2].c_str());
			float z = atof(tokens[3].c_str());

			xmin = std::min(x, xmin);
			xmax = std::max(x, xmax);
			ymin = std::min(y, ymin);
			ymax = std::max(y, ymax);
			zmin = std::min(z, zmin);
			zmax = std::max(z, zmax);

			vec3 position = { x, y, z };
			gPositions.push_back(position);
		}
		else if (tokens[0] == "vn")
		{
			float x = atof(tokens[1].c_str());
			float y = atof(tokens[2].c_str());
			float z = atof(tokens[3].c_str());
			vec3 normal = { x, y, z };
			gNormals.push_back(normal);
		}
		else if (tokens[0] == "f")
		{
			unsigned int a = face_index(tokens[1].c_str());
			unsigned int b = face_index(tokens[2].c_str());
			unsigned int c = face_index(tokens[3].c_str());

			vec3 triangle = { a - 1, b - 1, c - 1 };
			gTriangles.push_back(triangle);
		}
	}

	fin.close();

	printf("Loaded mesh from %s. (%lu vertices, %lu normals, %lu triangles)\n", fileName.c_str(), gPositions.size(), gNormals.size(), gTriangles.size());
	printf("Mesh bounding box is: (%0.4f, %0.4f, %0.4f) to (%0.4f, %0.4f, %0.4f)\n", xmin, ymin, zmin, xmax, ymax, zmax);
}



GLuint LoadShaders(const string& vertex_file_path, const string& fragment_file_path) {
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path.c_str(), ios::in);
	if (VertexShaderStream.is_open()) {
		stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		cout << "VertexShader file open errer";
		return 0;
	}

	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path.c_str(), ios::in);
	if (FragmentShaderStream.is_open()) {
		stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else {
		cout << "FragmentShader file open errer";
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Compile Fragment Shader
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void resize_callback(GLFWwindow*, int w, int h) {
	Width = w;
	Height = h;

	glViewport(0, 0, w, h);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

int main(int argc, char** argv) {
	GLFWwindow* window;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	window = glfwCreateWindow(Width, Height, "Phong Shader", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, resize_callback);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


	GLuint shaderProgram = LoadShaders("Phong.vert", "Phong.frag");

	// Bunny
	load_mesh("bunny.obj");

	int vertexNum = gPositions.size(), triangleNum = gTriangles.size();

	vector<vec3> Colors(vertexNum, vec3(1.0f, 1.0f, 1.0f));

	mat4 model = mat4(1.0f);

	const int numBuffers = 4;
	vector<GLuint> GLBuffers;
	GLBuffers.resize(numBuffers, 0);
	glGenBuffers(numBuffers, &GLBuffers[0]);

	//make indices
	vector<unsigned int> Indices;
	for (auto i : gTriangles) {
		Indices.push_back(i.x);
		Indices.push_back(i.y);
		Indices.push_back(i.z);
	}

	// Transform Matrix
	// 
	// Model
	mat4 modelScale(
		10, 0, 0, 0,
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 1
	);
	mat4 modeltranslate(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0.1, -1, -1.5, 1
	);
	mat4 modeling = modeltranslate * modelScale;

	vec3 e(0, 0, 0);
	vec3 u(1, 0, 0);
	vec3 v(0, 1, 0);
	vec3 w(0, 0, 1);

	// Camera
	mat4 camera(
		u.x, u.y, u.z, 0,
		v.x, v.y, v.z, 0,
		w.x, w.y, w.z, 0,
		e.x, e.y, e.z, 1
	);
	camera = inverse(camera);

	// Projection
	float l = -0.1;
	float r = 0.1;
	float b = -0.1;
	float t = 0.1;
	float n = 0.1;
	float f = 1000;

	mat4 projection(
		(2.0f * n) / (r - l), 0, 0, 0,
		0, (2.0f * n) / (t - b), 0, 0,
		(r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1,
		0, 0, -(2.0f * f * n) / (f - n), 0
	);

	init_timer();

	while (!glfwWindowShouldClose(window)) {

		start_timing();

		processInput(window);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		GLuint modelingLoc = glGetUniformLocation(shaderProgram, "modeling");
		GLuint cameraLoc = glGetUniformLocation(shaderProgram, "camera");
		GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, &modeling[0][0]);
		glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, &camera[0][0]);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

		GLuint cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");
		GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLuint laLoc = glGetUniformLocation(shaderProgram, "la");
		GLuint pLoc = glGetUniformLocation(shaderProgram, "p");
		GLuint acLoc = glGetUniformLocation(shaderProgram, "ac");
		GLuint dcLoc = glGetUniformLocation(shaderProgram, "dc");
		GLuint scLoc = glGetUniformLocation(shaderProgram, "sc");

		vec3 cameraPos(0, 0, 0);
		glUniform3fv(cameraPosLoc, 1, &cameraPos[0]);
		vec4 lightPos(-1, -1, -1, 0);
		glUniform4fv(lightPosLoc, 1, &lightPos[0]);
		vec3 la(0.2, 0.2, 0.2);
		glUniform3fv(laLoc, 1, &la[0]);
		float p = 0.0f;
		glUniform1f(pLoc, p);
		vec3 ac(0, 0, 0);
		vec3 dc(1, 1, 1);
		vec3 sc(0, 0, 0);
		glUniform3fv(acLoc, 1, &ac[0]);
		glUniform3fv(dcLoc, 1, &dc[0]);
		glUniform3fv(scLoc, 1, &sc[0]);

		glBegin(GL_TRIANGLES);

		for (int i = 0; i < triangleNum; i++) {
			for (int j = 0; j < 3; j++) {
				vec3 curNormal = gNormals[Indices[i*3+j]];
				vec3 pos = gPositions[Indices[i * 3 + j]];
				vec3 curColor(1, 1, 1);

				glNormal3f(curNormal.x, curNormal.y, curNormal.z);
				glVertex3f(pos.x, pos.y, pos.z);
				glColor3f(curColor.x, curColor.y, curColor.z);
			}
		}

		glEnd();

		// Timer
		float timeElapsed = stop_timing();
		gTotalFrames++;
		gTotalTimeElapsed += timeElapsed;
		float fps = gTotalFrames / gTotalTimeElapsed;
		char string[1024] = { 0 };
		sprintf(string, "OpenGL Bunny: %0.2f FPS", fps);
		glfwSetWindowTitle(window, string);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	if (GLBuffers[0] != 0) {
		glDeleteBuffers(numBuffers, &GLBuffers[0]);
	}
	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
