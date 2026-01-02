#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/glu.h>
#include <fstream>
#include "shader.hpp"
#include "sim.hpp"
#include "network.hpp"
#include <cstdlib>
#include <queue>
#include <windows.h>

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

//WGL context
HWND   g_hWnd   = nullptr;
HDC    g_hDC    = nullptr;
HGLRC  g_hGLRC  = nullptr;


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

bool init() {
    srand((unsigned int)time(nullptr));

    /* ---------------------------------------------------------
       1. Create hidden Win32 window
    --------------------------------------------------------- */
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "WGLWindowClass";

    if (!RegisterClass(&wc)) {
        fprintf(stderr, "Failed to register window class\n");
        return false;
    }

    g_hWnd = CreateWindow(
        wc.lpszClassName,
        "Hidden OpenGL Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        G_WIDTH, G_HEIGHT,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );

    if (!g_hWnd) {
        fprintf(stderr, "Failed to create window\n");
        return false;
    }

    g_hDC = GetDC(g_hWnd);

    /* ---------------------------------------------------------
       2. Set pixel format
    --------------------------------------------------------- */
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(g_hDC, &pfd);
    if (!pf || !SetPixelFormat(g_hDC, pf, &pfd)) {
        fprintf(stderr, "Failed to set pixel format\n");
        return false;
    }

    /* ---------------------------------------------------------
       3. Create temporary OpenGL context
    --------------------------------------------------------- */
    HGLRC tempContext = wglCreateContext(g_hDC);
    if (!tempContext || !wglMakeCurrent(g_hDC, tempContext)) {
        fprintf(stderr, "Failed to create temporary GL context\n");
        return false;
    }

    /* ---------------------------------------------------------
       4. Initialize GLEW (to get WGL extensions)
    --------------------------------------------------------- */
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW init failed: %s\n", glewGetErrorString(err));
        return false;
    }

    if (!wglCreateContextAttribsARB) {
        fprintf(stderr, "wglCreateContextAttribsARB not supported\n");
        return false;
    }

    /* ---------------------------------------------------------
       5. Create OpenGL 4.3 Core context
    --------------------------------------------------------- */
    int contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    g_hGLRC = wglCreateContextAttribsARB(g_hDC, 0, contextAttribs);
    if (!g_hGLRC) {
        fprintf(stderr, "Failed to create OpenGL 4.3 context\n");
        return false;
    }

    /* ---------------------------------------------------------
       6. Replace temporary context
    --------------------------------------------------------- */
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);
    wglMakeCurrent(g_hDC, g_hGLRC);

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
            
            Sleep(1000/60);
        }
    }
    printf("end\n");
    delete fSim;
    return 0;
}