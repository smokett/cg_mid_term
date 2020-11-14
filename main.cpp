// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.
#include "Model.h"

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Project includes
#include "maths_funcs.h"



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "cockroach_plane.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/



using namespace std;
GLuint shaderProgramID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

vector<int> mesh_count;
GLuint c_vp[8];
GLuint c_vn[8];
GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;
GLfloat rotate_y_1 = 0.0f;
GLfloat rotate_y_2 = 5.0f;
GLfloat rotate_z = 0.0f;
GLfloat translate_x = 0.0f,translate_y = 0.0f,translate_z = 0.0f;
GLfloat scale_x = 1.0f, scale_y = 1.0f, scale_z = 1.0f;
glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, -10.0f);
glm::vec3 camera_look_to = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 camera_up_direction = glm::vec3(0.0f, 1.0f, 0.0f);
GLfloat camera_x = 0.0f, camera_y = 0.0f, camera_z = -10.0f;
GLfloat yaw = 90.0f;
GLfloat pitch = 0.0f;
GLfloat camera_speed = 0.05f;
int mouse_last_x;
int mouse_last_y;
bool first_mouse = true;
DWORD deltaTime = 0.0f;
DWORD lastFrame = 0.0f;
std::vector<size_t> currentPoints;
std::vector<size_t> cockroachPoints;
int currentMeshNum = 0;
int cockroachMeshNum = 0;
bool animation0 = true;
bool animation1 = true;
GLfloat move_forward = 0.0f;

Model* cockRoach;




// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS




void display() {
	glBindVertexArray(cockRoach->vao);
	
	DWORD currentFrame = timeGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 R = glm::mat4(1.0f);
	glm::mat4 S = glm::mat4(1.0f);
	glm::mat4 M = glm::mat4(1.0f);

	T = glm::translate(glm::mat4(1.0f), glm::vec3(translate_x, translate_y, translate_z));
	//R = glm::rotate(glm::mat4(1.0f), glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
	S = glm::scale(glm::mat4(1.0f), glm::vec3(scale_x, scale_y, scale_z));
	M = T * R * S;

	glm::mat4 V = glm::mat4(1.0f);
	//glm::vec3 look_direction = glm::vec3(camera_x, 0.0f, camera_z);
	V = glm::lookAt(camera_position, camera_position + camera_look_to, camera_up_direction);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");


	// Root of the Hierarchy
	mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
	glm::mat4 model = glm::mat4(1.0f);
	model = M;

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(M));
	//size_t pointCount = cockroachPoints[0];
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[1]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[1]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[1]);
	//pointCount += cockroachPoints[1];



	// Set up the child matrix
	glm::mat4 right_leg_2 = glm::mat4(1.0f);
	right_leg_2 = glm::rotate(right_leg_2, glm::radians(rotate_y_2), glm::vec3(0, 1, 0));

	// Apply the root matrix to the child matrix
	right_leg_2 = model * right_leg_2;

	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(right_leg_2));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[2]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[2]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[2]);


	glm::mat4 right_leg_1 = glm::mat4(1.0f);
	right_leg_1 = glm::rotate(right_leg_1, glm::radians(rotate_y), glm::vec3(0, 1, 0));
	right_leg_1 = model * right_leg_1;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(right_leg_1));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[3]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[3]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[3]);

	glm::mat4 left_leg_1 = glm::mat4(1.0f);
	left_leg_1 = glm::rotate(left_leg_1, glm::radians(rotate_y), glm::vec3(0, 1, 0));
	left_leg_1 = model * left_leg_1;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(left_leg_1));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[4]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[4]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[4]);


	glm::mat4 left_leg_2 = glm::mat4(1.0f);
	left_leg_2 = glm::rotate(left_leg_2, glm::radians(rotate_y_2), glm::vec3(0, 1, 0));
	left_leg_2 = model * left_leg_2;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(left_leg_2));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[5]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[5]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[5]);


	glm::mat4  right_leg_0 = glm::mat4(1.0f);
	right_leg_0 = glm::rotate(right_leg_0, glm::radians(rotate_y_1), glm::vec3(0, 1, 0));
	right_leg_0 = model * right_leg_0;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(right_leg_0));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[6]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[6]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[6]);


	glm::mat4  left_leg_0 = glm::mat4(1.0f);
	left_leg_0 = glm::rotate(left_leg_0, glm::radians(rotate_y_1), glm::vec3(0, 1, 0));
	left_leg_0 = model * left_leg_0;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(left_leg_0));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[7]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[7]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[7]);


	glm::mat4  plane = glm::mat4(1.0f);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(plane));
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vp_vbo[0]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, cockRoach->vn_vbo[0]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, cockRoach->vertex_nums[0]);


	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	if (animation0) {
		rotate_y -= 40.0f * delta;
	}
	else {
		rotate_y += 40.0f * delta;
	}
	if (!animation0) {
		rotate_y_1 -= 40.0f * delta;
	}
	else {
		rotate_y_1 += 40.0f * delta;
	}

	if (animation1) {
		rotate_y_2 -= 40.0f * delta;
	}
	else {
		rotate_y_2 += 40.0f * delta;
	}
	if (rotate_y >= 10) {
		animation0 = true;
	}
	if (rotate_y <= -10) {
		animation0 = false;
	}
	if (rotate_y_2 >= 10) {
		animation1 = true;
	}
	if (rotate_y_2 <= -10) {
		animation1 = false;
	}

	rotate_y = fmodf(rotate_y, 360.0f);
	rotate_y_1 = fmodf(rotate_y_1, 360.0f);
	rotate_y_2 = fmodf(rotate_y_2, 360.0f);
	translate_z -= 0.5f * delta;

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array
	string filename = MESH_NAME;
	//generateObjectBufferMesh(filename);
	//cockroachMeshNum = currentMeshNum;
	//cockroachPoints = currentPoints;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
	cockRoach = new Model(filename, shaderProgramID);
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	float deltaT = deltaTime * 0.001f;
	camera_speed = 5.0f * deltaT;
	if (key == 'w') {
		camera_position += camera_speed * camera_look_to;
	}
	else if (key == 's') {
		camera_position -= camera_speed * camera_look_to;
	}
	else if (key == 'a') {
		camera_position -= glm::normalize(glm::cross(camera_look_to, camera_up_direction)) * camera_speed;
	}
	else if (key == 'd') {
		camera_position += glm::normalize(glm::cross(camera_look_to, camera_up_direction)) * camera_speed;
	}
}

void mousemove(int x, int y) {
	if (first_mouse)
	{
		mouse_last_x = x;
		mouse_last_y = y;
		first_mouse = false;
	}

	float xoffset = x - mouse_last_x;
	float yoffset = mouse_last_y - y;
	mouse_last_x = x;
	mouse_last_y = y;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera_look_to = glm::normalize(direction);
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");
	glutSetCursor(GLUT_CURSOR_NONE);

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutPassiveMotionFunc(mousemove);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
