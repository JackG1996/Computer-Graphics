// Std. Includes
#include <string>
// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "SOIL2/SOIL2.h"
#include <irrKlang.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations


#include <iostream>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"


// Properties
const GLuint WIDTH = 1600, HEIGHT = 900;
int SCREEN_WIDTH, SCREEN_HEIGHT;
using namespace irrklang;

ISoundEngine *SoundEngine = createIrrKlangDevice();
// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();

// Camera
Camera camera(glm::vec3(0.0f, 100.0f, 100.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System", glfwGetPrimaryMonitor(), nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);

	SoundEngine->play2D("audio/Adagio.mp3", GL_TRUE);

	Shader shader("modelLoadingVertex.txt", "modelLoadingFrag.txt");
	Shader skyboxShader("skyboxVertex.txt", "skyboxFrag.txt");


	Model earthModel("models/Earth.obj");
	Model moonModel("models/Moon.obj");
	Model marsModel("models/Mars 2K.obj");
	Model sunModel("models/inSun.obj");
	Model mercuryModel("models/Mercury 2K.obj");
	Model venusModel("models/Venus 2K.obj");
	Model jupiterModel("models/Jupiter 2K.obj");
	Model saturnModel("models/Saturn.obj");
	Model uranusModel("models/hoth.obj");
	Model neptuneModel("models/yavin-IV.obj");

	GLfloat skyboxVertices[] = {
		// Positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glBindVertexArray(0);

	//Load textures
	vector<const GLchar *>faces;
	faces.push_back("skybox/starfield_bk.tga");
	faces.push_back("skybox/starfield_dn.tga");
	faces.push_back("skybox/starfield_ft.tga");
	faces.push_back("skybox/starfield_lf.tga");
	faces.push_back("skybox/starfield_rt.tga");
	faces.push_back("skybox/starfield_up.tga");

	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	glm::mat4 projection(1);
	projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		DoMovement();

		// Clear the colorbuffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 view(1);
		view = camera.GetViewMatrix();

		shader.Use();

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 model(1);
		model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
		model = glm::rotate(model, (GLfloat)glfwGetTime() * 0.08f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		sunModel.Draw(shader); //sun

		glm::mat4 model1(1);
		model1 = glm::scale(model1, glm::vec3(0.5f, 0.5f, 0.5f));
		model1 = glm::rotate(model1, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		model1 = glm::translate(model1, glm::vec3(-34.0f, 0.0f, -16.0f));
		model1 = glm::rotate(model1, (GLfloat)glfwGetTime() * 0.3f, glm::vec3(1.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model1));
		mercuryModel.Draw(shader); // mercury

		glm::mat4 model2(1);
		
		model2 = glm::scale(model2, glm::vec3(0.8f, 0.8f, 0.8f));
		model2 = glm::rotate(model2, (GLfloat)glfwGetTime() * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
		model2 = glm::translate(model2, glm::vec3(50.0f, 0.0f, -32.0f));
		model2 = glm::rotate(model2, (GLfloat)glfwGetTime() * 0.3f, glm::vec3(1.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model2));
		venusModel.Draw(shader); // venus

		glm::mat4 model3(1);

		model3 = glm::rotate(model3, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		model3 = glm::translate(model3, glm::vec3(0.0f, 0.0f, -58.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model3));
		earthModel.Draw(shader);// earth

		glm::mat4 model4(1);

		model4 = glm::scale(model4, glm::vec3(0.006f, 0.006f, 0.006f));
		model4 = glm::rotate(model3 * model4, (GLfloat)glfwGetTime() * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
		model4 = glm::translate(model4, glm::vec3(13.0f, 0.0f, -67.0f));
		model4 = glm::rotate(model4, (GLfloat)glfwGetTime() * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model4));
		moonModel.Draw(shader); //moon
		
		glm::mat4 model5(1);

		model5 = glm::scale(model5, glm::vec3(0.6f, 0.6f, 0.6f));
		model5 = glm::rotate(model5, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		model5 = glm::translate(model5, glm::vec3(-35.0f, 0.0f, -120.0f));
		model5 = glm::rotate(model5, (GLfloat)glfwGetTime() * 0.3f, glm::vec3(1.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model5));
		marsModel.Draw(shader); // mars

		glm::mat4 model6(1);
		
		model6 = glm::scale(model6, glm::vec3(3.3f, 3.3f, 3.3f));
		model6 = glm::rotate(model6, (GLfloat)glfwGetTime() * 0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
		model6 = glm::translate(model6, glm::vec3(-21.0f, 0.0f, -30.0f));
		model6 = glm::rotate(model6, (GLfloat)glfwGetTime() * 0.3f, glm::vec3(3.0f, 0.0f, 2.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model6));
		jupiterModel.Draw(shader); //jupiter


		glm::mat4 model7(1);
		model7 = glm::scale(model7, glm::vec3(0.05f, 0.05f, 0.05f));
		model7 = glm::rotate(model7, (GLfloat)glfwGetTime() * 0.4f, glm::vec3(0.0f, 1.0f, 0.0f));
		model7 = glm::translate(model7, glm::vec3(-2000.0f, 0.0f, -6030.0f));
		model7 = glm::rotate(model7, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(2.0f, 3.0f, 3.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model7));
		saturnModel.Draw(shader); // saturn

		glm::mat4 model8(1);
		model8 = glm::scale(model8, glm::vec3(0.25f, 0.25f, 0.25f));
		model8 = glm::rotate(model8, (GLfloat)glfwGetTime() * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
		model8 = glm::translate(model8, glm::vec3(1550.0f, 0.0f, -1450.0f));
		model8 = glm::rotate(model8, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(1.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model8));
		uranusModel.Draw(shader); // uranus

		glm::mat4 model9(1);
		model9 = glm::scale(model9, glm::vec3(0.2f, 0.2f, 0.2f));
		model9 = glm::rotate(model9, (GLfloat)glfwGetTime() * 0.2f, glm::vec3(0.0f, 1.0f, 0.0f));
		model9 = glm::translate(model9, glm::vec3(0.0f, 0.0f, -2250.0f));
		model9 = glm::rotate(model9, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(1.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model9));
		neptuneModel.Draw(shader); // uranus

		
		//Lighting Information
		GLint objectColorLoc = glGetUniformLocation(shader.Program, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(shader.Program, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shader.Program, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shader.Program, "viewPos");
		glUniform3f(objectColorLoc, 0.3f, 0.5f, 1.0f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		
		
		// Draw skybox as last

		glDepthFunc(GL_LESS); // Set depth function back to default
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use();

		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// skybox cube
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}