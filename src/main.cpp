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

/**
 * DGEUB var
*/
int trans_y_axiz_speed = 0.2;




void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// screen size
unsigned int screen_height = 900;
unsigned int screen_width = 1200;

//physics
float velocity_player = -0.0f;
float increase_vel_by = 0.0005f;
int set_time_zero = 0.0f;



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
 * function methods
*/

void glDeleteAllOs(unsigned int VAO,unsigned int VBO,unsigned int EBO){
    glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

// gives a number 0 - max included
int randGenIntWithBound(int max){
    max++;
    return (float)rand() / RAND_MAX * max;
}
/**
 * level rendering
 * 
 * todo: player 
 * * velicoty 
 * * motion 
 *  - rendering
 * ! - colliions
 * ! - animation
 * ! - lighting 
 * 
 * todo: background
 *  * motion 
 *  - rendering
 *  - texture
 * 
 *  ? floor sperate from bg
 *  ? lighting on th e ceiling
 * 
 * todo: zapper horizontal
 *  - rendering
 *  
*/

// returns next level if die will give -1
int level(int level_number,Shader shader,GLFWwindow * window){
    // preprocessing
    int level_length = (9 + level_number * 6);
    int level_speed = 1 + level_number * 0.3;
    int number_of_zappers = (level_length) * 2 / 3;

    std::cout << "level_length : " << level_length << std::endl;
    std::cout << "level_speed : " << level_speed << std::endl;
    std::cout << "number_of_zappers : " << number_of_zappers << std::endl;

    // ZAPPERS
    int zapper_position[level_length];
    int zapper_position_above_or_below[level_length];
    int zapper_position_rotation[level_length];

    for (size_t i = 0; i < level_length; i++)
    {
        zapper_position[i] = 0;
        zapper_position_above_or_below[i] = 0;
        zapper_position_rotation[i] = 0;
    }
    


    // todo make it better
    for (size_t i = 1,x =0; i <= level_length - 2 && x <=  number_of_zappers ; i++)
    {
        if( i % 6 == 0){
            zapper_position[i-1] = 1;
            zapper_position[i] = 1;
            zapper_position[i+1] = 1;
            x += 3;
        }
    }    

    for (size_t i = 0; i < level_length; i++)
    {
        std::cout << zapper_position[i] << "|" ;
        if(zapper_position[i] == 1){
            zapper_position_above_or_below[i] = randGenIntWithBound(2);
            // std::cout << "postion a/b :" << zapper_position_above_or_below[i] << std::endl;
        }
    }
    std::cout << std::endl;

        for (size_t i = 0; i < level_length; i++)
    {
        if(zapper_position[i] == 1){
            zapper_position_rotation[i] =  45 * randGenIntWithBound(3);
            // std::cout << "rotation a/b :" << zapper_position_rotation[i] << std::endl;
        }
    }

    
    
    
    float vertices_player[] = {
		// positions		// colors			// texture coordinates
		-0.5f, -0.5f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-0.5f,  0.5f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 0.5f, -0.5f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 0.5f,  0.5f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
    float vertices_zap[] = {
		// positions		// colors			// texture coordinates
		-0.095f, -0.38f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-0.095f,  0.38f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 0.095f, -0.38f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 0.095f,  0.38f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
    float vertices_background[] = {
		// positions		// colors			// texture coordinates
		-1.0f, -1.0f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-1.0f,  1.0f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 1.0f, -1.0f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 1.0f,  1.0f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
    float vertices_coin[] = {
		// positions		// colors			// texture coordinates
		-1.0f, -1.0f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-1.0f,  1.0f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 1.0f, -1.0f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 1.0f,  1.0f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
	
    unsigned int indices[] = {
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

    unsigned int VBO_background, VAO_background, EBO_background;
    {
        // VBO, VAO_background, EBO_ground
        glGenBuffers(1, &VBO_background);
        glGenVertexArrays(1, &VAO_background);
        glGenBuffers(1, &EBO_background);

        // bind VAO_background
        glBindVertexArray(VAO_background);

        // bind VBO_background
        glBindBuffer(GL_ARRAY_BUFFER, VBO_background);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_background), vertices_background, GL_STATIC_DRAW);

        // put index array in EBO_ground
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_background);
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

    // COIN
    unsigned int VBO_coin, VAO_coin, EBO_coin;
    {
        // VBO, VAO_coin, EBO_ground
        glGenBuffers(1, &VBO_coin);
        glGenVertexArrays(1, &VAO_coin);
        glGenBuffers(1, &EBO_coin);

        // bind VAO_coin
        glBindVertexArray(VAO_coin);

        // bind VBO_coin
        glBindBuffer(GL_ARRAY_BUFFER, VBO_coin);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_coin), vertices_coin, GL_STATIC_DRAW);

        // put index array in EBO_ground
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_coin);
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

    unsigned int VBO_zap, VAO_zap, EBO_zap;
    {
        // VBO, VAO_zap, EBO_ground
        glGenBuffers(1, &VBO_zap);
        glGenVertexArrays(1, &VAO_zap);
        glGenBuffers(1, &EBO_zap);

        // bind VAO_zap
        glBindVertexArray(VAO_zap);

        // bind VBO_zap
        glBindBuffer(GL_ARRAY_BUFFER, VBO_zap);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_zap), vertices_zap, GL_STATIC_DRAW);

        // put index array in EBO_ground
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_zap);
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

    unsigned int texture_zap;
    {
        glGenTextures(1, &texture_zap);
        glBindTexture(GL_TEXTURE_2D, texture_zap);

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
        unsigned char* data = stbi_load("../src/assets/zapper_1.png", &width, &height, &nChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }

        stbi_image_free(data);
    }

    unsigned int texture_coin;
    {
        glGenTextures(1, &texture_coin);
        glBindTexture(GL_TEXTURE_2D, texture_coin);

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
        unsigned char* data = stbi_load("../src/assets/coin_1.png", &width, &height, &nChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }

        stbi_image_free(data);
    }


	shader.activate();
	shader.setInt("texture_player", 0);
	shader.setInt("texture_background", 1);
	shader.setInt("texture_zap", 2);
	shader.setInt("texture_coin", 3);




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
            for (int i = 0 ; i < level_length ; i++){
                // bind texture
                // glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture_background);
                glBindVertexArray(0);

                // transformation
                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() * level_speed  + (float)i*2 ,0.0f, 0.0f));
                trans = glm::scale(trans, glm::vec3(1.0f, 1.0f,1.0f));
                shader.setMat4("transform", trans);

                // draw shapes
                glBindVertexArray(VAO_background);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        // !for zapper motion and rendering texture and rendring 
        {
            // rending the entire length of the level
            for (int i = 0 ; i < level_length ; i++){
                if( zapper_position[i] == 1 ){
                    // bind texture
                    // glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture_zap);
                    glBindVertexArray(0);

                    // transformation
                    glm::mat4 trans = glm::mat4(1.0f);
                    trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() * level_speed + (float)i,(zapper_position_above_or_below[i]-1)*0.5f, 0.0f));
                    trans = glm::scale(trans, glm::vec3(0.7f, 0.7f,0.7f));
                    trans = glm::rotate(trans, glm::radians((float) zapper_position_rotation[i]), glm::vec3(0.0f, 0.0f, 1.0f));
                    shader.setMat4("transform", trans);

                    // draw shapes
                    glBindVertexArray(VAO_zap);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
            }
        }

		// !for player motion and rendering texture and rendring 
        {
            // bind texture
            // glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_player);
            glBindVertexArray(0);
            
            // transformation
            trans_y += velocity_player * (glfwGetTime() - set_time_zero);

            if( trans_y >= 1.5){
                trans_y = 1.5;
                set_time_zero = glfwGetTime();
                velocity_player = 0;
            }
            else if( trans_y <= 0){
                trans_y = 0;
                set_time_zero = glfwGetTime();
                velocity_player = 0;
            }



            glm::mat4 trans = glm::mat4(1.0f);
            trans = glm::translate(trans,glm::vec3(-0.75f,trans_y-0.75f, 1.0f));
            trans = glm::scale(trans, glm::vec3(0.13f, 0.3f,1.0f));
            shader.setMat4("transform", trans);

            // draw shapes
            glBindVertexArray(VAO_player);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }

        // !for coin motion and rendering texture and rendring 
        {
            // rending the entire length of the level
            for (int i = 0 ; i < level_length ; i++){
                // bind texture
                // glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture_coin);
                glBindVertexArray(0);

                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() * level_speed  + (float)i*2 ,0.0f, 0.0f));
                trans = glm::rotate(trans, glm::radians((float) zapper_position_rotation[i]), glm::vec3(0.0f, 0.0f, 1.0f));
                trans = glm::scale(trans, glm::vec3(0.07f, 0.07f,1.0f));
                shader.setMat4("transform", trans);

                // draw shapes
                glBindVertexArray(VAO_coin);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

        }




		// send new frame to window
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteAllOs(VAO_player,VBO_player,EBO_player);
	glDeleteAllOs(VAO_background,VBO_background,EBO_background);

    return 0;
}

/**
 * main method
*/

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

    // !CALLING THE LEVEL 
    level(2,shader,window);
    // level(3,shader,window);
    // level(4,shader,window);

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
		trans_y += trans_y_axiz_speed;
	}
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		trans_y -= trans_y_axiz_speed;
	}

    if(currentGameStatus == GAME_ACTIVE ){
        // jumping from floor

        // std:: cout << "set time zero : " << set_time_zero << "|" << glfwGetTime() << std :: endl;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if(set_time_zero <= glfwGetTime() + 0.2){
                // velocity_player += increase_vel_by * 1;
            }
            velocity_player += increase_vel_by * 2;
        }
        else{
            velocity_player -= increase_vel_by;
        }
    }
}
