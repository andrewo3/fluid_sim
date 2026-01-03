#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <fstream>
#include "shader.hpp"
#include "sim.hpp"
#include "network.hpp"
#include <cstdlib>
#include <queue>

#define FPS_BUFFER_LENGTH 3


void debugCallback(
GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    printf("GL DEBUG: %s\n", message);
}

/* Constants */
//Screen dimension constants
constexpr int kScreenWidth = 640;
constexpr int kScreenHeight = 480;

//Graphics program
ShaderProgram gProgram;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;
bool gRenderQuad = true;

//Create shaders
Shader vertexShader(GL_VERTEX_SHADER);
Shader fragmentShader(GL_FRAGMENT_SHADER);

//Fluid sim object
const int G_WIDTH = 256;
const int G_HEIGHT = 256;
uint8_t pixel_data[G_WIDTH * G_HEIGHT * 3];
Fluid* fSim;

//timing
unsigned long currentTime = 0;
unsigned long lastTime = 0;
float dt = 0.0;
long frames = 0;
std::deque<float> fps_meas;

//EGL Context
EGLDisplay display;
EGLContext context;
EGLSurface surface;


//options
bool framebyframe = false;
bool velocity_field = false;
bool rainbow = true;
bool fullscreen = false;
bool show_cursor = false;
bool automated = true;

bool initGL() {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallback, NULL);
    //Success flag
    bool success = true;
    
    //Generate program
    gProgram.init();
    //initialize shaders
    if (!vertexShader.init("shaders/vertex.glsl")) {
        printf("Unable to compile vertex shader %d!\n", vertexShader.id);
        printShaderLog(vertexShader.id);
        return false;
    }

    if (!fragmentShader.init("shaders/frag.glsl")) {
        printf("Unable to compile fragment shader %d!\n", fragmentShader.id);
        printShaderLog(fragmentShader.id);
        return false;
    }
    //Attach vertex shader to program
    gProgram.attachShader(&vertexShader);

    //Attach fragment shader to program
    gProgram.attachShader(&fragmentShader);
    //Link program
    if(!gProgram.link()) {
        printf("Error linking program %d!\n", gProgram.id);
        printProgramLog(gProgram.id);
        return false;
    }
    //Get vertex attribute location
    gVertexPos2DLocation = glGetAttribLocation( gProgram.id, "LVertexPos2D" );
    if( gVertexPos2DLocation == -1 )
    {
        printf( "LVertexPos2D is not a valid glsl program variable!\n" );
        return false;
    }else
    {
        //Initialize clear color
        glClearColor( 0.f, 0.f, 0.f, 1.f );

        //VBO data
        GLfloat vertexData[] =
        {
            -1.0f, -1.0f,0.0f,0.0f,
                1.0f, -1.0f,1.0f,0.0f,
                1.0f,  1.0f,1.0f,1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };

        //IBO data
        GLuint indexData[] = { 0, 1, 2, 2, 3, 0 };

        //create VAO
        glGenVertexArrays(1, &gVAO);
        glBindVertexArray(gVAO);

        //Create VBO
        glGenBuffers( 1, &gVBO );
        glBindBuffer( GL_ARRAY_BUFFER, gVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW );

        //Create IBO
        glGenBuffers( 1, &gIBO );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indexData, GL_STATIC_DRAW );

        //vertex attributes
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,(void*)0);
        glEnableVertexAttribArray(0);
        //tex coords
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*4,(void*)(sizeof(GLfloat)*2));
        glEnableVertexAttribArray(1);
    }
    return success;
}

/* Function Implementations */
bool init() {
    //Initialization flag
    srand(time(nullptr)); 

    // TODO: Create OpenGL Context with EGL
    display = eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL);
    if (display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        return false;
    }

    if (!eglInitialize(display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return false;
    }

    EGLConfig config;
    EGLint numConfigs;

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT, // request desktop OpenGL
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
    if (numConfigs < 1) {
        fprintf(stderr, "No suitable EGL configs found\n");
        return false;
    }

    surface = eglCreatePbufferSurface(display, config, (EGLint[]){
        EGL_WIDTH, G_WIDTH,
        EGL_HEIGHT, G_HEIGHT,
        EGL_NONE
    });

    if (surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return false;
    }

    eglBindAPI(EGL_OPENGL_API);

    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    eglMakeCurrent(display, surface, surface, context);

    if (context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        return false;
    }

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed: %s\n", glewGetErrorString(err));
        return false;
    }

    return true;
}

void close()
{
    //Destroy OpenGL Context
}

inline int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

int main(int argc, char** argv) {
    //Final exit code
    int exitCode = 0;
    //Initialize
    if(init() == false) {
        printf( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else
    {
        //The quit flag
        bool quit = false;
        fSim = new Fluid(G_WIDTH,G_HEIGHT,0.00001,0.00001);
        Connection conn = Connection("192.168.193.25",65432);
        conn.connect_();
        
        Mouse mouse(G_WIDTH,G_HEIGHT);
        
        //The event data
        //The main loop
        while (quit == false) {

            if (rainbow) {
                fSim->mouse_density = currentTime / 200;
                fSim->mouse_density %= fSim->densities;
            }
            
            currentTime = GetTicks();
            if (currentTime/1000 != lastTime/1000) {
                if (fps_meas.size() >= FPS_BUFFER_LENGTH) {
                    fps_meas.pop_front();
                }
                fps_meas.push_back(1000.0*frames/(float)(1000+currentTime%1000));
                frames = 0;
            }
            dt = (currentTime - lastTime)/1000.0;
            lastTime = currentTime;

            //run sim frame
            mouse.update();
            if (!framebyframe) {
                fSim->simStep(&mouse,dt);
            }

            //Update the surface
            frames++;
            float avg = 0;
            for (float f: fps_meas) {
                avg += f;
            }
            avg /= fps_meas.size();

            glBindTexture(GL_TEXTURE_2D, fSim->mixedOut);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
            conn.sendFrame(pixel_data, G_WIDTH*G_HEIGHT*3);
            usleep(1000000/60);
        }
    }
    printf("end\n");
    delete fSim;
    return 0;
}