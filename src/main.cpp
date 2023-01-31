#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <unistd.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H
 
#define d(x) std::cout<<#x<<" = "<< x <<"\n"
#define s(x) std::cout<<#x<<" = "<< x <<"|"
#define n()  std::cout<<"\n"
#define max(x,y) (x > y) ? x : y
#define min(x,y) (x < y) ? x : y
 
/**
 * DGEUB var
*/
float trans_y_axiz_speed = 0.2;
float dbg_speed = 0.005;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// screen size
unsigned int screen_height = 900;
unsigned int screen_width = 1200;

//physics
float velocity_player = -0.0f;
float increase_vel_by = 0.0005f;
float set_time_zero = 0.0f;



// random
bool pause_game;
float dev = 0.0;


// level details
int level_length;
float level_speed;
int number_of_zappers;



// Variables that help the translation of object
static GLfloat trans_x = -0.75f, trans_y = -0.75f, trans_z = 1.0f;

// game status
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
};

// game status
enum PlayerStatus {
    walking,
    flying,
    falling
    // jumping,
};
int playerIs = walking;


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

// array coords
float coords_player[4][2];
float coords_coin[4][2];
float coords_zapper[4][2];

// player coords
void getPlayer4Coords(){
    // s(trans_x);
    // s(trans_y);
    // n();

    coords_player[0][0]=trans_x+ 0.04f;
    coords_player[0][1]=trans_y+ 0.05f;

    coords_player[1][0]=trans_x- 0.04f;
    coords_player[1][1]=trans_y+ 0.05f - dev;

    coords_player[2][0]=trans_x- 0.04f;
    coords_player[2][1]=trans_y- 0.05f;

    coords_player[3][0]=trans_x+ 0.04f;
    coords_player[3][1]=trans_y- 0.05f;

    if(0){
        s(coords_player[1][0]);s(coords_player[1][1]);s("---------");s(coords_player[0][0]);d(coords_player[0][1]);
        n();n();n();
        s(coords_player[2][0]);s(coords_player[2][1]);s("---------");s(coords_player[3][0]);d(coords_player[3][1]);
    }
}

// zapper coords

void getZapper4Coords(float x,float y){
    // s(x);
    // s(y);
    // n();

    coords_zapper[0][0]=x+ 0.05f;
    coords_zapper[0][1]=y+ 0.14f;

    coords_zapper[1][0]=x- 0.05f;
    coords_zapper[1][1]=y+ 0.14f;

    coords_zapper[2][0]=x- 0.05f;
    coords_zapper[2][1]=y- 0.14f;

    coords_zapper[3][0]=x+ 0.05f;
    coords_zapper[3][1]=y- 0.14f;

    if(0){
        s(coords_zapper[1][0]);s(coords_zapper[1][1]);s("---------");s(coords_zapper[0][0]);s(coords_zapper[0][1]);
        n();n();n();
        s(coords_zapper[2][0]);s(coords_zapper[2][1]);s("---------");s(coords_zapper[3][0]);s(coords_zapper[3][1]);
    }
}

// coin coords

void getCoin4Coords(float x,float y){
    // s(x);
    // s(y);
    // n();

    coords_coin[0][0]=x+ 0.02f;
    coords_coin[0][1]=y+ 0.02f - dev;

    coords_coin[1][0]=x- 0.02f;
    coords_coin[1][1]=y+ 0.02f - dev;

    coords_coin[2][0]=x- 0.02f;
    coords_coin[2][1]=y- 0.02f - dev;

    coords_coin[3][0]=x+ 0.02f;
    coords_coin[3][1]=y- 0.02f - dev;

    if(0){
        s(coords_coin[1][0]);s(coords_coin[1][1]);s("---------");s(coords_coin[0][0]);d(coords_coin[0][1]);
        n();n();n();
        s(coords_coin[2][0]);s(coords_coin[2][1]);s("---------");s(coords_coin[3][0]);d(coords_coin[3][1]);
    }
}

bool isPointInsideRect(float quad[4][2],float x,float y) {
    float d1, d2, d3, d4;

    d1 = (x - quad[1][0]) * (quad[0][1] - quad[1][1]) - (quad[0][0] - quad[1][0]) * (y - quad[1][1]);
    d2 = (x - quad[2][0]) * (quad[1][1] - quad[2][1]) - (quad[1][0] - quad[2][0]) * (y - quad[2][1]);
    d3 = - (x - quad[3][0]) * (quad[2][1] - quad[3][1]) +  (quad[2][0] - quad[3][0]) * (y - quad[3][1]);
    d4 = - (x - quad[0][0]) * (quad[3][1] - quad[0][1]) + (quad[3][0] - quad[0][0]) * (y - quad[0][1]);

    // n();
    bool flag =(d1 > 0 && d2 > 0 && d3 > 0 && d4 > 0) || (d1 < 0 && d2 < 0 && d3 < 0 && d4 < 0); ;
    if(flag){
        // s(d1 * d3);s(d2 * d4);
        d("-----------------------------------------------");
        // s(x);d(y);
        // for (size_t i = 0; i < 4; i++)
        // {
        //     for (size_t j = 0; j < 2; j++)
        //     {
        //         /* code */
        //         std::cout << quad[i][j] <<"," ;
        //     }
        //     std::cout << "|" ;
        // }
        // std::cout << "\n" ;
        d("-----------------------------------------------");
        n();
    }
    return (flag);
}

bool collisionDetect(float quad1[4][2],float quad2[4][2]){
    // for point of quad 1 in quad 2
    
    if(1){
        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 2; j++)
            {
                /* code */
                std::cout << quad1[i][j] <<" " ;
            }
            std::cout << "\t" ;
        }
        std::cout << "\n" ;
        for (size_t i = 0; i < 4; i++)
        {
            for (size_t j = 0; j < 2; j++)
            {
                /* code */
                std::cout << quad2[i][j] <<" " ;
            }
            std::cout << "\t" ;
        }
        std::cout << "\n" ;
    }

    bool is_inside_2 = false;
    for (size_t i = 0; i < 4; i++)
    {
        is_inside_2 |= isPointInsideRect(quad2,quad1[i][0],quad1[i][0]);
    }

    // d(is_inside_2);

    // for point of quad 1 in quad 2
    bool is_inside_1 = false;
    for (size_t i = 0; i < 4; i++)
    {
        is_inside_1 |= isPointInsideRect(quad1,quad2[i][0],quad2[i][0]);
    }

    // d(is_inside_1);

    // d((is_inside_2 || is_inside_1));

    return is_inside_2 && is_inside_1;
}

bool checkCoinCollision(int i,float y,float time){
    getPlayer4Coords();
    getCoin4Coords(-1.0f * time * level_speed  + (float)i/5,y);

    // s("player");s(coords_player[0][0]);s(coords_player[0][1]);n();
    // s("coin");s(coords_coin[0][0]);s(coords_coin[0][1]);n();
    // s("coin");s(-1.0f * time * level_speed  + (float)i/2);s(y);n();

    return collisionDetect(coords_player,coords_coin);
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
    level_length = (9 + level_number * 6);
    level_speed = 0.4 + level_number * 0.15;
    level_speed = 0;
    number_of_zappers = (level_length) * 2 / 3;

    std::cout << "level_length : " << level_length << std::endl;
    std::cout << "level_speed : " << level_speed << std::endl;
    std::cout << "number_of_zappers : " << number_of_zappers << std::endl;

    // ZAPPERS
    float coin_x_position[level_length];
    float coin_y_position[level_length];
    float zapper_position[level_length];
    float zapper_position_above_or_below[level_length];
    float zapper_position_rotation[level_length];

    for (size_t i = 0; i < level_length; i++)
    {
        coin_x_position[i] = 0;
        coin_y_position[i] = 0;

        zapper_position[i] = 0;
        zapper_position_above_or_below[i] = 0;
        zapper_position_rotation[i] = 0;
    }

    coin_x_position[1] = 1;
    coin_y_position[1] = 0.0f;
    
    // todo make it better
    for (size_t i = 1,x =0; i <= level_length - 2 && x <=  number_of_zappers ; i++)
    {
        if( i % 3 == 0){
            // coin_x_position[i] = 1;
            // coin_y_position[i] = 0.75f * randGenIntWithBound(2) - 0.75f;
        }
        if( i % 6 == 5){
            zapper_position[i] = 1;
            x += 3;
        }
        if( i % 6 == 0){
            zapper_position[i] = 1;
            x += 3;
        }
        if( i % 6 == 1){
            zapper_position[i] = 1;
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
        std::cout << "<" << coin_x_position[i] << "|" << coin_x_position[i] <<">" ;
        if(zapper_position[i] == 1){
            zapper_position_rotation[i] =  45 * randGenIntWithBound(3);
            // std::cout << "rotation a/b :" << zapper_position_rotation[i] << std::endl;
        }
    }
    std::cout << std::endl;


    
    
    
    float vertices_player[] = {
		// positions		// colors			// texture coordinates
		-0.08f, -0.1f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-0.08f,  0.1f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 0.08f, -0.1f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 0.08f,  0.1f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
	};
    float vertices_zap[] = {
		// positions		// colors			// texture coordinates
		-0.10f, -0.28f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-0.10f,  0.28f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 0.10f, -0.28f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 0.10f,  0.28f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
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
		-0.04f, -0.04f,  0.0f,	1.0f, 1.0f, 0.5f,	0.0f, 0.0f,	// bottom left
		-0.04f,  0.04f,  0.0f,	0.5f, 1.0f, 0.75f,	0.0f, 1.0f,	// top left
		 0.04f, -0.04f,  0.0f,	0.6f, 1.0f, 0.2f,	1.0f, 0.0f,	// bottom right
		 0.04f,  0.04f,  0.0f,	1.0f, 0.2f, 1.0f,	1.0f, 1.0f	// top right
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
	// walking
	unsigned int texture_player_walking[4];
    {
        {
            glGenTextures(1, &texture_player_walking[0]);
            glBindTexture(GL_TEXTURE_2D, texture_player_walking[0]);

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
            unsigned char* data = stbi_load("../src/assets/player/1.running/_0.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/1.running" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_walking[1]);
            glBindTexture(GL_TEXTURE_2D, texture_player_walking[1]);

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
            unsigned char* data = stbi_load("../src/assets/player/1.running/_1.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/1.running" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_walking[2]);
            glBindTexture(GL_TEXTURE_2D, texture_player_walking[2]);

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
            unsigned char* data = stbi_load("../src/assets/player/1.running/_2.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/1.running" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_walking[3]);
            glBindTexture(GL_TEXTURE_2D, texture_player_walking[3]);

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
            unsigned char* data = stbi_load("../src/assets/player/1.running/_3.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/1.running" << std::endl;
            }

            stbi_image_free(data);
        }
    }

    unsigned int texture_player_flying[4];
    {
        {
            glGenTextures(1, &texture_player_flying[0]);
            glBindTexture(GL_TEXTURE_2D, texture_player_flying[0]);

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
            unsigned char* data = stbi_load("../src/assets/player/2.flying/_0.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/2.flying" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_flying[1]);
            glBindTexture(GL_TEXTURE_2D, texture_player_flying[1]);

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
            unsigned char* data = stbi_load("../src/assets/player/2.flying/_1.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/2.flying" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_flying[2]);
            glBindTexture(GL_TEXTURE_2D, texture_player_flying[2]);

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
            unsigned char* data = stbi_load("../src/assets/player/2.flying/_2.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/2.flying" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_flying[3]);
            glBindTexture(GL_TEXTURE_2D, texture_player_flying[3]);

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
            unsigned char* data = stbi_load("../src/assets/player/2.flying/_3.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/2.flying" << std::endl;
            }

            stbi_image_free(data);
        }
    }

    unsigned int texture_player_falling[2];
    {
        {
            glGenTextures(1, &texture_player_falling[0]);
            glBindTexture(GL_TEXTURE_2D, texture_player_falling[0]);

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
            unsigned char* data = stbi_load("../src/assets/player/6.falling/_0.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/3.falling" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_player_falling[1]);
            glBindTexture(GL_TEXTURE_2D, texture_player_falling[1]);

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
            unsigned char* data = stbi_load("../src/assets/player/6.falling/_1.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from player/3.falling" << std::endl;
            }

            stbi_image_free(data);
        }
    }



    // dying 
    // todo 

    // ZAPPERS
	unsigned int texture_zapper[5];
    {
        {
            glGenTextures(1, &texture_zapper[0]);
            glBindTexture(GL_TEXTURE_2D, texture_zapper[0]);

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
            unsigned char* data = stbi_load("../src/assets/zapper/zapper1.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from zapper" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_zapper[1]);
            glBindTexture(GL_TEXTURE_2D, texture_zapper[1]);

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
            unsigned char* data = stbi_load("../src/assets/zapper/zapper2.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from zapper" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_zapper[2]);
            glBindTexture(GL_TEXTURE_2D, texture_zapper[2]);

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
            unsigned char* data = stbi_load("../src/assets/zapper/zapper3.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from zapper" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_zapper[3]);
            glBindTexture(GL_TEXTURE_2D, texture_zapper[3]);

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
            unsigned char* data = stbi_load("../src/assets/zapper/zapper4.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from zapper" << std::endl;
            }

            stbi_image_free(data);
        }

         {
            glGenTextures(1, &texture_zapper[4]);
            glBindTexture(GL_TEXTURE_2D, texture_zapper[4]);

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
            unsigned char* data = stbi_load("../src/assets/zapper/zapper5.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from zapper" << std::endl;
            }

            stbi_image_free(data);
        }
    }


     // ZAPPERS
	unsigned int texture_coin[8];
    {
        {
            glGenTextures(1, &texture_coin[0]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[0]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin1.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_coin[1]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[1]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin2.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_coin[2]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[2]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin3.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_coin[3]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[3]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin4.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

         {
            glGenTextures(1, &texture_coin[4]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[4]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin5.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_coin[5]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[5]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin6.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_coin[6]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[6]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin7.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }

        {
            glGenTextures(1, &texture_coin[7]);
            glBindTexture(GL_TEXTURE_2D, texture_coin[7]);

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
            unsigned char* data = stbi_load("../src/assets/coins/coin8.png", &width, &height, &nChannels, 0);

            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture from coins" << std::endl;
            }

            stbi_image_free(data);
        }
    }


    // bg
    unsigned int texture_bg_1[1];
    {
        glGenTextures(1, &texture_bg_1[0]);
        glBindTexture(GL_TEXTURE_2D, texture_bg_1[0]);

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
        unsigned char* data = stbi_load("../src/assets/bg/Background1/Background1.png", &width, &height, &nChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture from bg" << std::endl;
        }

        stbi_image_free(data);
    }


	shader.activate();
	// shader.setInt("texture_player", 0);
	// shader.setInt("texture_background", 1);
	// shader.setInt("texture_zap", 2);
	// shader.setInt("texture_coin", 3);



    int coin_frame_counter = 0;
    int zapper_frame_counter = 0;
    int player_walking_frame_counter = 0;
    int player_flying_frame_counter = 0;
    int player_falling_frame_counter = 0;
    int count  = 0;



	while (!glfwWindowShouldClose(window)) {
		// process input
		processInput(window);

        //pause_game
        if(pause_game){
            double time_at_pause = glfwGetTime();
            std::cout<< "game is paused ||" << std::endl;

            getPlayer4Coords();

            sleep(5);
            // n();
            glfwSetTime(time_at_pause);
            std::cout<< "game is resumed |>" << std::endl;
        }

        // std:: cout << glfwGetTime() << std:: endl;

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
                glBindTexture(GL_TEXTURE_2D, texture_bg_1[0]);
                glBindVertexArray(0);

                // transformation
                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() * level_speed  + (float)i*2 ,0.0f, 0.0f));
                // trans = glm::scale(trans, glm::vec3(1.0f, 1.0f,1.0f));
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
                    if(zapper_frame_counter >= 30 * 4){
                        zapper_frame_counter = 0;
                    }else{
                        zapper_frame_counter ++;
                    }
                    glBindTexture(GL_TEXTURE_2D, texture_zapper[zapper_frame_counter/30]);
                    glBindVertexArray(0);

                    // transformation
                    glm::mat4 trans = glm::mat4(1.0f);
                    trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() * level_speed + (float)i,(zapper_position_above_or_below[i]-1)*0.5f, 0.0f));
                    // trans = glm::scale(trans, glm::vec3(0.7f, 0.7f,0.7f));
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

            // motion of player
            // trans_y += velocity_player * (glfwGetTime() - set_time_zero);

            if( trans_y >= 0.75){
                trans_y = 0.75;
                set_time_zero = glfwGetTime();
                velocity_player = 0;
            }
            else if( trans_y <= -0.75){
                trans_y = -0.75;
                set_time_zero = glfwGetTime();
                velocity_player = 0;
                playerIs = walking;
            }
            else{
            }



            if(playerIs == walking){
                if(player_walking_frame_counter >= 7 * 3){
                    player_walking_frame_counter = 0;
                }else{
                    player_walking_frame_counter ++;
                }

                glBindTexture(GL_TEXTURE_2D, texture_player_walking[player_walking_frame_counter/7]);
                glBindVertexArray(0);
                
                // transformation

                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(trans_x,trans_y,trans_z));
                // trans = glm::scale(trans, glm::vec3(0.1f, 0.2f,1.0f));
                shader.setMat4("transform", trans);

                // draw shapes
                glBindVertexArray(VAO_player);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            if(playerIs == flying){
                if(player_flying_frame_counter >= 7 * 3){
                    player_flying_frame_counter = 0;
                }else{
                    player_flying_frame_counter ++;
                }

                glBindTexture(GL_TEXTURE_2D, texture_player_flying[player_flying_frame_counter/7]);
                glBindVertexArray(0);
                
                // transformation

                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(trans_x,trans_y,trans_z));
                // trans = glm::scale(trans, glm::vec3(0.1f, 0.2f,1.0f));
                shader.setMat4("transform", trans);

                // draw shapes
                glBindVertexArray(VAO_player);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            if(playerIs == falling){
                if(player_falling_frame_counter >= 7 * 1){
                    player_falling_frame_counter = 0;
                }else{
                    player_falling_frame_counter ++;
                }

                glBindTexture(GL_TEXTURE_2D, texture_player_falling[player_falling_frame_counter/7]);
                glBindVertexArray(0);
                
                // transformation

                glm::mat4 trans = glm::mat4(1.0f);
                trans = glm::translate(trans,glm::vec3(trans_x,trans_y,trans_z));
                // trans = glm::scale(trans, glm::vec3(0.1f, 0.2f,1.0f));
                shader.setMat4("transform", trans);

                // draw shapes
                glBindVertexArray(VAO_player);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

        }

        // !for coin motion and rendering texture and rendring 
        {
            // rending the entire length of the level
            for (int i = 0 ; i < level_length ; i++){
                if(coin_x_position[i] == 1){
                    // bind texture
                    // glActiveTexture(GL_TEXTURE0);
                    if(coin_frame_counter >= 5 * 7){
                        coin_frame_counter = 0;
                    }else{
                        coin_frame_counter ++;
                    }
                    glBindTexture(GL_TEXTURE_2D, texture_coin[coin_frame_counter/5]);
                    glBindVertexArray(0);

                    glm::mat4 trans = glm::mat4(1.0f);
                    trans = glm::translate(trans,glm::vec3(-1.0f * glfwGetTime() * level_speed  + (float)i/5 ,coin_y_position[i], 0.0f));
                    // trans = glm::scale(trans, glm::vec3(0.07f, 0.07f,1.0f));
                    shader.setMat4("transform", trans);

                    // draw shapes
                    glBindVertexArray(VAO_coin);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
            }

        }

        // // ! collion detect

        // // checking coins
        for (size_t i = 0; i < level_length ; i++)
        {
            if(coin_x_position[i] == 1){
                int time = glfwGetTime();
                if (checkCoinCollision(i,coin_y_position[i],time) == true){
                    std::cout << "coin collision detected !!!" << ++count << std::endl;
                    coin_x_position[i] == 0;
                    coin_y_position[i] == 0;
                }
                else{
                    // std::cout << "---------\n";
                }
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
    level(1,shader,window);
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
		trans_y += dbg_speed;
	}
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		trans_y -= dbg_speed;
	}
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		trans_x -= dbg_speed;
	}
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		trans_x += dbg_speed;
	}
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		pause_game = !pause_game;
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
            playerIs = flying;
        }
        else{
            velocity_player -= increase_vel_by;
            playerIs = falling;
        }
    }
}
