#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
glm::mat4 trans(1.0f);
glm::mat4 trans2(1.0f);
bool snd = false;
GLuint shaderProgramID;
// Vertex Shader (for convenience, it is defined in the main here, but we will be using text files for shaders in future)
// Note: Input to this shader is the vertex positions that we specified for the triangle. 
// Note: gl_Position is a special built-in variable that is supposed to contain the vertex position (in X, Y, Z, W)
// Since our triangle vertices were specified as vec3, we just set W to 1.0.
static const char* pVS = "                                                    \n\
#version 330																	\n\
                                                                              \n\
in vec3 vPosition;															  \n\
in vec4 vColor;																  \n\
out vec4 color;																 \n\
uniform mat4 transform;											              \n\
uniform mat4 transform2;	                                                   \n\
void main()                                                                     \n\
{																				\n\
	 																			\n\
    gl_Position = transform2 * transform * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);  \n\
																			 \n\
	color = vColor;																	\n\
}";

// Fragment Shader
// Note: no input in this shader, it just outputs the colour of all fragments, in this case set to red (format: R, G, B, A).
static const char* pFS = "                                              \n\
#version 330                                                            \n\
in vec4 color;                                                                     \n\
out vec4 FragColor;                                                      \n\
                                                                         \n\
void main()                                                               \n\
{                                                                         \n\
	FragColor = vec4(color.r,color.g,color.b,1.0);					\n\
}";

// Shader Functions- click on + to expand 
#pragma region SHADER_FUNCTIONS
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
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
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, pVS, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, pFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint generateObjectBuffer(GLfloat vertices[], GLfloat colors[]) {
	GLuint numVertices = 6;
	// Genderate 1 generic buffer object, called VBO
	GLuint VBO;
	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices * 14 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 6 * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, numVertices * 6 * sizeof(GLfloat), numVertices * 8 * sizeof(GLfloat), colors);
	return VBO;
}

void linkCurrentBuffertoShader(GLuint shaderProgramID) {
	GLuint numVertices = 6;
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices * 6 * sizeof(GLfloat)));
}
#pragma endregion VBO_FUNCTIONS


void display() {

	// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	if (snd == true)
	{
		glDrawArrays(GL_TRIANGLES, 3, 3);
	}

	glutSwapBuffers();
}


void init()
{
	GLfloat vertices[] = { 0.5f,0.0f,0.0f,
							0.0f,0.45f,0.0f,
						   -0.5f,0.0f,0.0f,
							0.0f,-0.5f,0.0f,
							0.9f,-0.5f,0.0f,
							0.45f,0.5f,0.0f };


	// Create a color array that identfies the colors of each vertex (format R, G, B, A)
	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f };

	// Set up your objects and shaders
	GLuint shaderProgramID = CompileShaders();
	// Put the vertices and colors into a vertex buffer object
	generateObjectBuffer(vertices, colors);
	// Link the current buffer to the shader
	linkCurrentBuffertoShader(shaderProgramID);
}

void keyboard(unsigned char c, int x, int y)
{

	shaderProgramID = CompileShaders();

	switch (c)
	{
		//Translations
	case 'w':
		trans = glm::scale(trans, glm::vec3(0.8, 0.8, 0.8));
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
		break;
	case 'q':
		trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));
		break;
	case 'e':
		trans = glm::translate(trans, glm::vec3(0.5f, 0.5f, 0.0f));
		break;
	case 'r':
		trans = glm::translate(trans, glm::vec3(-0.5f, -0.5f, 0.0f));
		break;
	case 'l':
		trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.5f));
		break;


		//Rotations
	case 'z':
		trans = glm::rotate(trans, glm::radians(65.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		break;
	case 'x':
		trans = glm::rotate(trans, glm::radians(165.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		break;
	case 'y':
		trans = glm::rotate(trans, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		break;

		//Uniform and Non-Uniform Scaling
	case 's':
		trans = glm::scale(trans, glm::vec3(2.0, 2.0, 2.0));
		break;
	case 'n':
		trans = glm::scale(trans, glm::vec3(0.5, 2.0, 0.5));
		break;

		//Combination
	case 'c':
		trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		trans = glm::rotate(trans, glm::radians(53.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		trans = glm::translate(trans, glm::vec3(1.2f, -1.0f, 0.0f));
		break;

	case 't':
		snd = true;
		glm::mat4 trans(1.0f);
		trans = glm::scale(trans, glm::vec3(0.8, 0.8, 0.8));
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
		trans2 = glm::scale(trans2, glm::vec3(0.8, 0.8, 0.8));
		trans2 = glm::translate(trans2, glm::vec3(0.0f, 0.5f, 0.0f));
		break;
	}
	unsigned int transformLoc = glGetUniformLocation(shaderProgramID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
	unsigned int transformLoc2 = glGetUniformLocation(shaderProgramID, "transform2");
	glUniformMatrix4fv(transformLoc2, 1, GL_FALSE, glm::value_ptr(trans2));
	glutPostRedisplay();


}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1200, 800);


	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	// Create 3 vertices that make up a triangle that fits on the viewport 
	init();
	glutKeyboardFunc(keyboard);
	// Begin infinite event loop
	glutMainLoop();

	return 0;
}

