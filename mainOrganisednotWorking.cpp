#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>

float pi = 3.1416;

#define d(x) cout<<#x<<" = "<<x<<endl
#define s(x) cout<<#x<<" = "<<x<<"|"
#define max(x,y) (x > y) ? x : y
#define min(x,y) (x < y) ? x : y

using namespace std;
//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||



/**
 * !variable
*/


// The Width of the screen
unsigned int SCREEN_WIDTH = 800;
// The height of the screen
unsigned int SCREEN_HEIGHT = 600;
const char * title = "LitPak CryRide";
static GLfloat trans_x = 0.f, trans_y = 0.f, trans_z = 0.f;
float speed = 0.2f;


/**
 * init methods
*/
void initGLFW(unsigned int version_major,unsigned int version_minor) {
    glfwInit();

    // pass in window params
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

// create window
void createWindow(GLFWwindow*& window,const char* title, unsigned int width,unsigned int height) {
    window = glfwCreateWindow(width,height,title,NULL,NULL); 

    if (!window){
        return;
    }  

    glfwMakeContextCurrent(window);    
}

// load glad lib
bool loadGlad(){
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

/**
 * shader methods
*/

// read file
string readFile(const char * fileName) {
    ifstream file;
    stringstream buf;

    string ret = "";

    // open file
    file.open(fileName);

    if (file.is_open()){
        // read buff
        buf << file.rdbuf();
        ret = buf.str();
    }
    else{
        cout << "ERROR cannot open file : " << fileName << endl;
    }

    file.close();

    return ret;


}

// gen shader file
int genShader(const char * filePath,GLenum type) {
    string shaderSrc = readFile(filePath);

    const GLchar * shader = shaderSrc.c_str();

    // building and compiling
    int shaderObj = glCreateShader(type);
    glShaderSource(shaderObj,1,&shader,NULL);
    glCompileShader(shaderObj);

    // for error correction
    int success;
    char infoLog[512];
    glGetShaderiv(shaderObj,GL_COMPILE_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(shaderObj,512,NULL,infoLog);
        cout << "ERROR : in shader compilation  : " << infoLog << endl ;
        return -1;
    }

    return shaderObj;
}

// gen shader program
int genShaderProgram(const char * vertexShaderPath,const char* fragmentShaderPath) {
    int shaderProgram = glCreateProgram();

    // ! no such function
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
        cout << "ERROR : in shader linking : " << infoLog << endl ;
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
} 

//bind shader
void bindShader(int shaderProgram) {
    // sets the current shader program to this shaderProgram
    glUseProgram(shaderProgram);
}

// set projection
void setOrthographicProjection(int shaderProgram,float left, float right,float bottom,float top,float near,float far) {
    // todo need to see how to do this properly of rno wleave it
    // ! i think i will not risk by moving the camera
    // for now 
}

//delete shader
void deleteShader(int shaderProgram) { 
    glDeleteProgram(shaderProgram);
} 


/**
 * VAO VBO methods
*/

//genrate VAO
void genVAO(VAO * vao) {
    glGenVertexArrays(1,&vao->val);
    glBindVertexArray(vao->val);
}

// gen buffer of certain type and set of data
template<typename T>
void genBufferObject(GLuint& bo,GLenum type,GLuint noElements,T* data,GLenum usage) {
    glGenBuffers(1, &bo);
    glBindBuffer(type, bo);
    glBufferData(type, noElements * sizeof(T), data, usage);
}

// update data in buffer object
template<typename T>
void updateData(GLuint& bo,GLintptr offset,GLuint noElements,T* data) {
    glBindBuffer(GL_ARRAY_BUFFER, bo);
    glBufferSubData(GL_ARRAY_BUFFER, offset, noElements * sizeof(T), data);
}

// set attribute pointers
template<typename T>
void setAttPointer(GLuint& bo, GLuint idx, GLint size, GLenum type, GLuint stride, GLuint offset, GLuint divisor = 0) {
    glBindBuffer(GL_ARRAY_BUFFER, bo);
    glVertexAttribPointer(idx, size, type, GL_FALSE, stride * sizeof(T), (void*)(offset * sizeof(T)));
    glEnableVertexAttribArray(idx);
    if (divisor > 0) {
        // reset _idx_ attribute every _divisor_ iteration through instances
        glVertexAttribDivisor(idx, divisor);
    }
}

//draw vao
void draw(VAO vao,GLenum mode , GLuint count, GLenum type , GLuint indices,GLuint instanceCount = 1) {
    glBindVertexArray(vao.val);
    glDrawElementsInstanced(mode, count, type,(void *) indices,instanceCount);
}

// unbind a buffer
void unbindBuffer(GLenum type) {
    glBindBuffer(type, 0);
}

// unbind vao
void unbindVAO() {
    glBindVertexArray(0);
}

// deallocate VAO/VBO memeory
void cleanup(VAO vao) {
    glDeleteBuffers(1, &vao.posVBO);
    glDeleteBuffers(1, &vao.offsetVBO);
    glDeleteBuffers(1, &vao.sizeVBO);
    glDeleteBuffers(1, &vao.EBO);
    glDeleteVertexArrays(1, &vao.val);
}


/**
 * cleanup method
*/

// terminate glfw
void cleanup() {
    glfwTerminate();
}

/**
 * main loop method 
*/


// process input 
void processInput(GLFWwindow * window, int k, int s, int action, int mods ) {
    if ( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ){
        glfwSetWindowShouldClose(window,true);
    }
}

// clear screen
void clearScreen() {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// new frame
void newFrame(GLFWwindow* window){
    glfwSwapBuffers(window);
    glfwPollEvents();
}




// new 

/**
 * !MAIN
*/

// callback for window size change
void framebufferSizeCallback(GLFWwindow* window,int width,int height) {
    glViewport(0,0,width,height);
}

int main(){

    //timing 
    double dt = 0.0;
    double lastFrame = 0.0;

    // init
    initGLFW(3,3);

    //create window
    GLFWwindow* window = nullptr;
    createWindow(window,title,SCREEN_WIDTH,SCREEN_HEIGHT);

    // error handling
    if(window == NULL){
        cout << "ERROR : wind was not created" << endl;
        cleanup();
        return -1;
    }

    //load glad
    if(!loadGlad()){
        cout << "ERROR could not init glad " << endl;
        cleanup();
        return -1;
    }

    // Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x 800, y = 800
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // ! what does this do
    glfwSetKeyCallback(window,processInput);

    // ! high chance of error in this
    GLuint shaderProgram = genShaderProgram("vertex.shader","fragment.shader");
     
    // setup vertex data
    float vertices_square[] = {
        // x     y      z
             0.5f, 0.5f,0.0f,1.0f,0.0f,2.0f,
            -0.5f, 0.5f,0.0f,1.0f,0.0f,2.0f,
            -0.5f,-0.5f,0.0f,1.0f,0.0f,2.0f,
        // x     y      z
            -0.5f,-0.5f,0.0f,1.0f,0.0f,2.0f,
             0.5f,-0.5f,0.0f,1.0f,0.0f,2.0f,
             0.5f, 0.5f,0.0f,1.0f,0.0f,2.0f
    };

    // setup vao and vbo
    unsigned int VBO, VAO;
    // genVAO(&vao); 

    // !change no of vertices before render
    int no_of_vertices = 6 ; // since drawing a square

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    //pos VBO
    genBufferObject<float>(vao.posVBO,GL_ARRAY_BUFFER,2 * no_of_vertices, vertices_square ,GL_STATIC_DRAW);
    setAttPointer<float>(vao.posVBO,0,2,GL_FLOAT,2,0);

    // offset VBO
    genBufferObject<float>(vao.offsetVBO,GL_ARRAY_BUFFER,1 * 2 , offsets , GL_DYNAMIC_DRAW);
    setAttPointer<float>(vao.offsetVBO,1,2,GL_FLOAT,2,0,1);

    // size VBO
    genBufferObject<float>(vao.sizeVBO,GL_ARRAY_BUFFER,1 * 2 , offsets , GL_DYNAMIC_DRAW);
    setAttPointer<float>(vao.sizeVBO,2,2,GL_FLOAT,2,0,1);

    // EBO
    genBufferObject<int>(vao.EBO,GL_ELEMENT_ARRAY_BUFFER, 3 * 2 , indices , GL_STATIC_DRAW);

    // unbind vbo and vao
    unbindBuffer(GL_ARRAY_BUFFER);
    unbindVAO();

    // render loop 
    while (!glfwWindowShouldClose(window)){
        //update time

        dt = glfwGetTime() - lastFrame;
        lastFrame += dt;

        // input
        // !is this needed !!!!
        // processInput(window);

        // clear screen for new frame
        clearScreen();

        // render object 
        bindShader(shaderProgram);

        // !change before rendering
        int no_of_triangles = 2;
        draw(vao,GL_TRIANGLES , 3 * no_of_triangles , GL_UNSIGNED_INT,0);

        newFrame(window);
    }

    cleanup(vao);
    deleteShader(shaderProgram);
    cleanup();

    return 0;
}

