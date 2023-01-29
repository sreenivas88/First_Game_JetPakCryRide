#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <math.h>

float pi = 3.1416;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int createVertexforNFace(float vertices[],int n,int arr_offset,float,int,int);
int createVertexforCFace(float vertices[],int n,int arr_offset,float,int,int);
int createVertexforWall(float vertices[],int n,int arr_offset,float,int,int);
static void key(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput( GLFWwindow* window);

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


// shader code
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "// for color RGB 3 data point color\n"
    "layout (location = 1) in vec3 aColor;\n"

    "out vec3 color;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    
    // "const float rimStart = 0.5f;\n"
    // "const float rimEnd = 1.0f;\n"
    // "const float rimMultiplier = 0.0f;\n"
    // "vec3  rimColor = vec3(1.0f,1.0f, 1.0f);\n" 

    
    "void main()\n"
    "{\n"
    // "   gl_Position =  proj * view * model * vec4(aPos, 1.0);\n"
    "   gl_Position = model * view * proj * vec4(aPos, 1.0);\n"
    "   color = aColor;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 color;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(color,1.0f);\n"
    "}\n\0";


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

// gen shader file
int genShader(const char * shaderSource,GLenum type) {
    // building and compiling
    int shaderObj = glCreateShader(type);
    glShaderSource(shaderObj,1,&shaderSource,NULL);
    glCompileShader(shaderObj);

    // for error correction
    int success;
    char infoLog[512];
    glGetShaderiv(shaderObj,GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shaderObj,512,NULL,infoLog);
        std::cout << "ERROR : in shader compilation  : " << infoLog << std::endl ;
        return -1;
    }

    return shaderObj;
}

// gen shader program
int genShaderProgram(const char * vertexShaderPath,const char* fragmentShaderPath) {
    int shaderProgram = glCreateProgram();

    // build and compile our shader program
    // ------------------------------------
    int vertexShader = genShader(vertexShaderPath,GL_VERTEX_SHADER);
    int fragmentShader = genShader(fragmentShaderPath,GL_FRAGMENT_SHADER);

    if( vertexShader == -1 || fragmentShader == -1){
        return -1;
    }

    // link shaders
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram) ;

    // for error correction
    int success;
    char infoLog[512];
    glGetShaderiv(shaderProgram,GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shaderProgram,512,NULL,infoLog);
        std::cout << "ERROR : in shader linking : " << infoLog << std::endl ;
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void genAndBinding(float vertices[],int size_vertices,unsigned int& VAO,unsigned int& VBO,GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, void *pointer){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_vertices, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    /**
     * !(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    */
    glVertexAttribPointer(index, size, type, normalized, stride,pointer);
    
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 
} 


int main()
{
    std::cout << "the game is rendering ...... " << std::endl;

    initGLFW();

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);

    //Load GLAD so it configures OpenGL
	gladLoadGL();

    // Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x 800, y = 800
	glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    // link shaders
    GLuint shaderProgram = genShaderProgram(vertexShaderSource,fragmentShaderSource);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    
    float vertices_player[] = {
        0.25f,0.25f,0.0f,     color_arr[1][0],color_arr[1][1],color_arr[1][2],
        -0.25f,0.25f,0.0f,    color_arr[1][0],color_arr[1][1],color_arr[1][2],
        -0.25f,-0.25f,0.0f,color_arr[1][0],color_arr[1][1],color_arr[1][2],

        -0.25f,-0.25f,0.0f,color_arr[0][0],color_arr[0][1],color_arr[0][2],
        0.25f,-0.25f,0.0f,color_arr[0][0],color_arr[0][1],color_arr[0][2],
        0.25f,0.25f,0.0f,color_arr[0][0],color_arr[0][1],color_arr[0][2]
    };

    float vertices_zapper[] = {
        0.25f,0.25f,0.0f,     color_arr[2][0],color_arr[2][1],color_arr[2][2],
        -0.25f,0.25f,0.0f,    color_arr[2][0],color_arr[2][1],color_arr[2][2],
        -0.25f,-0.25f,0.0f,   color_arr[2][0],color_arr[2][1],color_arr[2][2],
        
        -0.25f,-0.25f,0.0f, color_arr[2][0],color_arr[2][1],color_arr[2][2],
        0.25f,-0.25f,0.0f,  color_arr[2][0],color_arr[2][1],color_arr[2][2],
        0.25f,0.25f,0.0f,   color_arr[2][0],color_arr[2][1],color_arr[2][2]
    };

    // for square
    unsigned int VBO[2], VAO;
    genAndBinding(vertices_player,sizeof(vertices_player),VAO,VBO[0],1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // genAndBinding(vertices_zapper,sizeof(vertices_zapper),VAO,VBO[1],1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST); 

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    bool accelration_upwards = false;

    // -----------
    while (!glfwWindowShouldClose(window))
    {
		// Initializes matrices so they are not the null matrix
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 proj = glm::mat4(1.0f);


        // process input
		processInput(window);

        //update time


        // accelrating the object

        // direction

        if(acceleration > 0 && accelration_upwards == false){
            std::cout << " going up !!: " << std::endl;
            time_of_thrust = glfwGetTime();
            accelration_upwards = true;
        } 
        else if(acceleration < 0 && accelration_upwards == true){
            std::cout << " going down !!: " << std::endl;
            time_of_thrust = glfwGetTime();
            accelration_upwards = false;
        }
         
        float dispalacement_due_to_acceration = 0.5f * (acceleration) * (glfwGetTime() - time_of_thrust) * (glfwGetTime() - time_of_thrust);
        trans_y += dispalacement_due_to_acceration;

        if( trans_y >= 0.75){
            trans_y = 0.75;
            time_of_thrust = glfwGetTime();
            // std::cout << "hit ceil : " << (acceleration_gravity+acceleration_jetpak) << std::endl;
        }
        else if( trans_y <= -0.75){
            trans_y = -0.75;
            time_of_thrust = glfwGetTime();
            // acceleration = 0;
            // std::cout << "hit floar : " << (acceleration_gravity+acceleration_jetpak) << std::endl;
        }
        else{
            // time_of_thrust = glfwGetTime();
            // std::cout << "in air : " << (acceleration_gravity+acceleration_jetpak) << std::endl;
        }
        // making gravity 

        view = glm::translate(view, glm::vec3(trans_x ,0 ,0));
        view = glm::translate(view, glm::vec3( 0,trans_y,0));
        view = glm::translate(view, glm::vec3(0 ,0,trans_z ));

		// proj = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

		// Outputs the matrices into the Vertex Shader
		int modelLoc = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		int viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		int projLoc = glGetUniformLocation(shaderProgram, "proj");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        glClearColor(0.2f, 0.6f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
       
        // draw our first triangle
        glUseProgram(shaderProgram);


        // for cylinder 
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 4 * 3 ); // number of vertices here 


        // glBindVertexArray(0); // no need to unbind it every time 
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO[0]);
    // glDeleteBuffers(1, &VBO[1]);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int createWallFaces(float vertices[],int n,int arr_offset,int z_offset){
    return 0;
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
