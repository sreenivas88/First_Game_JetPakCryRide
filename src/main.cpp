#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

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

// screen size
unsigned int screen_height = 900;
unsigned int screen_width = 1200;

//timing 
double dt = 0.0;
double lastFrame = 0.0;

//physics
float speed = 0.01f;
float acceleration = -0.0f;
float time_of_thrust = 0.0f;


// Variables that help the translation of object
static GLfloat trans_x = 0.f, trans_y = 0.f, trans_z = 0.f;
int level_lenght = 10;
int level_speed = 0.7;

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



int main() {
	int success;
	char infoLog[512];

	std::cout << "Hello, world!" << std::endl;

	glfwInit();

	// openGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

# ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COPMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "OpenGL Tutorial", NULL, NULL);
	if (window == NULL) { // window not created
		std::cout << "Could not create window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, screen_width, screen_height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// SHADERS===============================
	Shader shader("../src/vertex.shader", "../src/fragment.shader");

	float vertices_player[] = {
		// positions		// colors			// texture coordinates
		-0.5f, -0.5f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-0.5f,  0.5f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 0.5f, -0.5f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 0.5f,  0.5f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
	unsigned int indices[] = {
		0, 1, 2, // first triangle
		3, 1, 2  // second triangle
	};

    float vertices_background[] = {
		// positions		// colors			// texture coordinates
		-2.0f, -2.0f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-2.0f,  2.0f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 2.0f, -2.0f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 2.0f,  2.0f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
	unsigned int indices_background[] = {
		0, 1, 2, // first triangle
		3, 1, 2  // second triangle
	};

	unsigned int VBO_player, VAO_player, EBO_player;
    {
        // VBO, VAO_player, EBO_player
        glGenBuffers(1, &VBO_player);
        glGenVertexArrays(1, &VAO_player);
        glGenBuffers(1, &EBO_player);

        // bind VAO_player
        glBindVertexArray(VAO_player);

        // bind VBO_player
        glBindBuffer(GL_ARRAY_BUFFER, VBO_player);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_player), vertices_player, GL_STATIC_DRAW);

        // put index array in EBO_player
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_player);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // set attributes pointers
        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    unsigned int VBO_background, VAO_background, EBO_ground;
    {
        // VBO, VAO_background, EBO_ground
        glGenBuffers(1, &VBO_background);
        glGenVertexArrays(1, &VAO_background);
        glGenBuffers(1, &EBO_ground);

        // bind VAO_background
        glBindVertexArray(VAO_background);

        // bind VBO_background
        glBindBuffer(GL_ARRAY_BUFFER, VBO_background);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_player), vertices_player, GL_STATIC_DRAW);

        // put index array in EBO_ground
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // set attributes pointers
        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

	// TEXTURES___________________________________________________________________________________________

    // PLAYER 
	// generate texture
	unsigned int texture_player;
    {
        glGenTextures(1, &texture_player);
        glBindTexture(GL_TEXTURE_2D, texture_player);

        // image wrap (s, t, r) = (x, y, z)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // border color
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // image filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // scale up -> blend colors
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // load image 1
        int width, height, nChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load("../src/assets/player_1.png", &width, &height, &nChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }

        stbi_image_free(data);
    }

    unsigned int texture_background;
    {
        glGenTextures(1, &texture_background);
        glBindTexture(GL_TEXTURE_2D, texture_background);

        // image wrap (s, t, r) = (x, y, z)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // border color
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // image filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // scale up -> blend colors
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // load image 1
        int width, height, nChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load("../src/assets/background_1.jpg", &width, &height, &nChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }

        stbi_image_free(data);
    }


	

	// glGenTextures(1, &texture_background);
	// glBindTexture(GL_TEXTURE_2D, texture_background);

	shader.activate();
	shader.setInt("texture_player", 0);
	shader.setInt("texture_background", 1);

	// transformation

	while (!glfwWindowShouldClose(window)) {
		// process input
		processInput(window);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
        shader.activate();


        // !for background motion and rendering texture and rendring 
        {
            // rending the entire length of the level
            for (int i = 0 ; i < level_lenght ; i++){
                // bind texture
                // glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture_background);
                glBindVertexArray(0);

                // transformation
                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() + (float)i ,0.0f, 0.0f));
                trans = glm::scale(trans, glm::vec3(2.0f, 2.0f,1.0f));
                shader.setMat4("transform", trans);

                // draw shapes
                glBindVertexArray(VAO_background);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

		// !for player motion and rendering texture and rendring 
        {
            // bind texture
            // glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_player);
            glBindVertexArray(0);
            
            // transformation
            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::translate(trans,glm::vec3(-0.75f,trans_y-0.75f, 1.0f));
            trans = glm::scale(trans, glm::vec3(0.13f, 0.3f,1.0f));
            shader.setMat4("transform", trans);

            // draw shapes
            glBindVertexArray(VAO_player);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }




		// send new frame to window
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO_player);
	glDeleteVertexArrays(1, &VBO_player);
	glDeleteBuffers(1, &EBO_player);

	glDeleteVertexArrays(1, &VAO_background);
	glDeleteVertexArrays(1, &VBO_background);
	glDeleteBuffers(1, &EBO_player);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput( GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
        glfwSetWindowShouldClose(window,true);
	}
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		trans_y += speed;
	}
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		trans_y -= speed;
	}

    if(currentGameStatus == GAME_ACTIVE ){
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            acceleration = 0.1;
        }
        else{
            acceleration = -0.1;
        }
    }
}
