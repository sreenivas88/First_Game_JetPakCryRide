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

// settings
const unsigned int width = 1000;
const unsigned int height = 1000;

//color settings
float color_arr[3][6] = {   {1.0,0.0,2.0,1.5,0.5,1.2},
                            {0.0,0.0,2.0,0.5,0.5,1.2},
                            {0.5f, 0.35f, 0.05f,0.5f, 0.35f, 0.05f,}
                            };

//int painting diff color 
int painted_face = 0;
int painted_wall = 0;

// Variables that help the rotation of object
float rotation = 0.0f,rotate_offset = 10.0f;
static GLfloat trans_x = 0.f, trans_y = 0.f, trans_z = 0.f;
int view_con = 0;



glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// Adjust the speed of the camera and it's sensitivity when looking around
float speed = 0.2f;
float sensitivity = 100.0f;

// shader code
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "// for color RGB 3 data point color\n"
    "layout (location = 1) in vec3 aColor;\n"

    "out vec3 color;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    
    "const float rimStart = 0.5f;\n"
    "const float rimEnd = 1.0f;\n"
    "const float rimMultiplier = 0.0f;\n"
    "vec3  rimColor = vec3(1.0f,1.0f, 1.0f);\n" 

    
    "void main()\n"
    "{\n"
    "   gl_Position =  proj * view * model * vec4(aPos, 1.0);\n"
    "   color = aColor;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 color;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(color,1.0f);\n"
    "}\n\0";

int main()
{
    int n = 3; 
    std::cout << "Enter number of sides : " << std::endl;
    std:: cin >> n;

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
	// gladLoadGL();
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x 800, y = 800
	glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    
    float vertices_cyc[90000] ;//= { cylinder
    float vertices_con[90000] ;//= { cone
    int ret,ret1 = 0;

    // front face 
    ret = createVertexforNFace(vertices_cyc,n,0,1.0f,0,1);
    ret1 = createVertexforNFace(vertices_con,n,0,1.0f,0,1);
    
    std :: cout << "Created face 1 with "<< ret << " is created\n";

    // back face 
    ret = createVertexforNFace(vertices_cyc,n,ret,-1.0f,0,0);

    // Wall face 
    ret = createVertexforWall(vertices_cyc,n,ret,-1.0f,0,2);
    ret = createVertexforWall(vertices_cyc,n,ret,1.0f,1,2);

    // cone slanting surface
    ret1 = createVertexforCFace(vertices_con,n,ret1,+1.0f,0,2);


    std :: cout << "Created face 2 with "<< ret << " is created\n";

    // for cylinder
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cyc), vertices_cyc, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    // for cone
    unsigned int VBO_c, VAO_c;
    glGenVertexArrays(1, &VAO_c);
    glGenBuffers(1, &VBO_c);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO_c);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_c);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_con), vertices_con, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 


    // Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST); 

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
		// Initializes matrices so they are not the null matrix
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 proj = glm::mat4(1.0f);

        // cameraFront = glm::vec3(-cameraPos);
		// Assigns different transformations to each matrix
		model = glm::rotate(model, glm::radians(rotation), glm::vec3(1.0f, 1.0f, 0.0f));
		view = glm::translate(view, glm::vec3(trans_x + 0.0f,trans_y -0.0f,trans_z -05.0f));
        view = view * glm::lookAt(cameraPos, - cameraPos + glm::vec3(trans_x ,trans_y ,trans_z ), cameraUp);
		proj = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

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


        if(view_con){
            // for cone
            glBindVertexArray(VAO_c); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            glDrawArrays(GL_TRIANGLES, 0, 6 * n  ); // number of vertices here 
        }
        else{
            // for cylinder 
            glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            glDrawArrays(GL_TRIANGLES, 0, 12 * n  ); // number of vertices here 
        }

        // glBindVertexArray(0); // no need to unbind it every time 
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO_c);
    glDeleteBuffers(1, &VBO_c);
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

int createVertexforNFace(float vertices[],int n,int vertices_already_created ,float z_axis,int twist_bool,int color){
    int k = 1;  // id around unit circle
    int triangles = 0;
    int no_of_vertices = vertices_already_created;

    while (triangles < n){
        //    first vetex is always origin 
        vertices[no_of_vertices++] = 0;
        vertices[no_of_vertices++] = 0;
        vertices[no_of_vertices++] = z_axis ; 

        // color
        vertices[no_of_vertices++] = color_arr[color][0];
        vertices[no_of_vertices++] = color_arr[color][1];
        vertices[no_of_vertices++] = color_arr[color][2];

        vertices[no_of_vertices++] =  0.5 * cos((k-1) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k-1) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = z_axis;

        // color
        vertices[no_of_vertices++] = color_arr[color][3];
        vertices[no_of_vertices++] = color_arr[color][4];
        vertices[no_of_vertices++] = color_arr[color][5]; 

        vertices[no_of_vertices++] = 0.5 * cos((k) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = z_axis;
        
        // color
        vertices[no_of_vertices++] = color_arr[color][3];
        vertices[no_of_vertices++] = color_arr[color][4];
        vertices[no_of_vertices++] = color_arr[color][5]; 

        k++;
        triangles++;
        
    }
    painted_face++;
    return no_of_vertices ;
}


int createVertexforCFace(float vertices[],int n,int vertices_already_created ,float z_axis,int twist_bool,int color){
    int k = 1;  // id around unit circle
    int triangles = 0;
    int no_of_vertices = vertices_already_created;

    while (triangles < n){
        //    first vetex is always origin 
        vertices[no_of_vertices++] = 0;
        vertices[no_of_vertices++] = 0;
        vertices[no_of_vertices++] = -1 * z_axis ; 

        // color
        vertices[no_of_vertices++] = color_arr[color][0];
        vertices[no_of_vertices++] = color_arr[color][1];
        vertices[no_of_vertices++] = color_arr[color][2];

        vertices[no_of_vertices++] =  0.5 * cos((k-1) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k-1) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = z_axis;

        // color
        vertices[no_of_vertices++] = color_arr[color][3];
        vertices[no_of_vertices++] = color_arr[color][4];
        vertices[no_of_vertices++] = color_arr[color][5]; 

        vertices[no_of_vertices++] = 0.5 * cos((k) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = z_axis;

        // color
        vertices[no_of_vertices++] = color_arr[color][3];
        vertices[no_of_vertices++] = color_arr[color][4];
        vertices[no_of_vertices++] = color_arr[color][5]; 

        k++;
        triangles++;
        
    }
    painted_face++;
    return no_of_vertices ;
}


int createVertexforWall(float vertices[],int n,int vertices_already_created ,float z_axis,int fitting,int color){
    int k = 1;  // id around unit circle
    int triangles = 0;
    int no_of_vertices = vertices_already_created;
    int twist_bool = 0;

    while (triangles < n){
        //    first vetex is always origin 
        vertices[no_of_vertices++] =  0.5 * cos((k-1 + fitting) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k-1 + fitting) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = -1 * z_axis ; 

        // color
        vertices[no_of_vertices++] = color_arr[color][0] ;
        vertices[no_of_vertices++] = color_arr[color][1];
        vertices[no_of_vertices++] = color_arr[color][2] + ((1 - painted_wall) * 0.24);

        vertices[no_of_vertices++] =  0.5 * cos((k-1) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k-1) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = z_axis;

        // color
        vertices[no_of_vertices++] = color_arr[color][3] ;
        vertices[no_of_vertices++] = color_arr[color][4];
        vertices[no_of_vertices++] = color_arr[color][5] + (( painted_wall) * 0.24); 

        vertices[no_of_vertices++] = 0.5 * cos((k) * (2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = 0.5 * sin((k) *(2 * pi / n) + twist_bool * pi/4);
        vertices[no_of_vertices++] = z_axis;

        // color
        vertices[no_of_vertices++] = color_arr[color][3] ;
        vertices[no_of_vertices++] = color_arr[color][4];
        vertices[no_of_vertices++] = color_arr[color][5] + (( painted_wall) * 0.24);

        k++;
        triangles++;
        
    }
    painted_wall = 1 - painted_wall;
    return no_of_vertices ;
}


int createWallFaces(float vertices[],int n,int arr_offset,int z_offset){
    return 0;
}

void key( GLFWwindow* window, int k, int s, int action, int mods )
{
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		   rotation += rotate_offset;
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		trans_z += speed;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		trans_z -= speed;
	}
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		trans_y += speed;
	}
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		trans_y -= speed;
	}
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		trans_x -= speed;
	}
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		trans_x += speed;
	}
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		view_con = 1 - view_con;
	}
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraPos -= glm::vec3(0, 1, 0) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraPos -= glm::vec3(0, -1, 0) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraPos -= glm::vec3(1, 0, 0) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraPos -= glm::vec3(-1, 0, 0) * speed;
    }
}
