#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
std::string loadShaderSrc(const char* filename);


// settings
const unsigned int width = 1000;
const unsigned int height = 1000;

//timing 
double dt = 0.0;
double lastFrame = 0.0;

//physics
float speed = 0.01f;
float acceleration = -0.0f;
float time_of_thrust = 0.0f;


// Variables that help the translation of object
static GLfloat trans_x = 0.f, trans_y = 0.f, trans_z = 0.f;

// game status
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
};

int currentGameStatus = GAME_ACTIVE;
// !need to chage for game over message and menue

//color settings
float color_arr[3][3] = {   {1.0,0.0,2.0},  // pink
                            {0.0,0.0,2.0},  //blue
                            {1.0,0.7,0.1}   // brown
                            };


/**
 * !functions wtih boulerplate code
*/

// glfw: initialize and configure
void initGLFW(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

// to load glad
int gladLoadGL(){
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    return 0;
}

/**
 * !main method
*/


int main() {
	int success;
	char infoLog[512];

	std::cout << "Hello, world!" << std::endl;

	initGLFW();

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Tutorial", NULL, NULL);
	if (window == NULL) { // window not created
		std::cout << "Could not create window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	gladLoadGL();

	glViewport(0, 0, 600, 800);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// SHADERS===============================
	Shader shader("../src/vertex.shcmaader", "../src/fragment.shader");

	float vertices[] = {
		// positions			// colors			// texture
		-0.25f, -0.5f, 0.0f,		1.0f, 1.0f, 0.5f,	0.0f, 0.0f,				// bottom left
		-0.25f,  0.5f, 0.0f,		0.5f, 1.0f, 0.75f,	0.0f, 1.0f,				// top left
		 0.25f, -0.5f, 0.0f,		0.6f, 1.0f, 0.2f,	1.0f, 0.0f,				// bottom right
		 0.25f,  0.5f, 0.0f,		1.0f, 0.2f, 1.0f,	1.0f, 1.0f				// top right
	};
	unsigned int indices[] = {
		0, 1, 2, // first triangle
		3, 1, 2  // second triangle
	};

	// VBO, VAO, EBO
	unsigned int VBO, VAO, EBO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &EBO);

	// bind VAO
	glBindVertexArray(VAO);

	// bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// put index array in EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// set attributes pointers===================================================================
	
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 
	
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6  * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Texture
	unsigned int texture1;

	glGenTextures(1,&texture1);
	glBindTexture(GL_TEXTURE_2D,texture1);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	
	// !checkput what does this do
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//load image
	int width_player,height_player,nChannels_player;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("../src/assets/player_1.png",&width_player,&height_player,&nChannels_player,0);

	if(data){
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width_player,height_player,0,GL_RGB,GL_UNSIGNED_BYTE,data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else{
		std::cout << "EORROR : texture not loaded" << std::endl;
	}

	stbi_image_free(data);

	shader.activate();
	shader.setInt("texture1",0);



	glm::mat4 trans = glm::mat4(1.0f);
	// trans = glm::rotate(trans, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	// trans = glm::scale(trans, glm::vec3(0.5f, 1.5f, 0.5f));
	// trans = glm::translate(trans, glm::vec3(trans_x + 0.0f,trans_y -0.0f,trans_z -05.0f));

	shader.activate();
	shader.setMat4("transform", trans);

	while (!glfwWindowShouldClose(window)) {
		// process inputGL_RGB
		processInput(window);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,texture1);

		// draw shapes
		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		shader.activate();
		// set color
		// float timeValue = glfwGetTime();
		// float blueValue = (sin(timeValue) / 2.0f) + 0.5f;
		// shader.set4Float("ourColor", 0.0f, 0.0f, blueValue, 1.0f);
		// trans = glm::rotate(trans, glm::radians(timeValue / 100), glm::vec3(0.1f, 0.1f, 0.1f));
		// shader.setMat4("transform", trans);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//trans = glm::translate(trans, glm::vec3(0.5f, 0.5f, 0.0f));
		//shader.setMat4("transform", trans);
		// draw second rectangle
		/*glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		trans = glm::translate(trans, glm::vec3(-0.5f, -0.5f, 0.0f));
		shader.setMat4("transform", trans);*/

		glBindVertexArray(0);

		// send new frame to window
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}