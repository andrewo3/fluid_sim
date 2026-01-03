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

#define FPS 24

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

//PBO
#define NUM_PBOS 3
GLuint pbo[NUM_PBOS];
static int pbo_index = 0;

//options
bool framebyframe = false;
bool velocity_field = false;
bool rainbow = true;
bool fullscreen = false;
bool show_cursor = false;
bool automated = true;

bool initGL() {
    //glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallback, NULL);

    glGenBuffers(NUM_PBOS, pbo);
    size_t size = G_WIDTH * G_HEIGHT * 3;
    for (int i = 0; i < NUM_PBOS; i++) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, size, nullptr, GL_STREAM_READ);
        glBufferStorage(GL_PIXEL_PACK_BUFFER, size, nullptr, GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    return true;
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
        initGL();
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
            dt = 1.0/FPS;
            float observed_dt = (currentTime - lastTime) / 1000.0f;
            //printf("FPS: %.2f\n", 1.0f / observed_dt);
            lastTime = currentTime;
            

            //run sim frame
            mouse.update();
            if (!framebyframe) {
                int times = 1;
                for (int i = 0; i < times; i++) {
                    fSim->simStep(&mouse,dt/times);
                }
            }
            //send frame
            int next = (pbo_index + 1) % NUM_PBOS;
            int read = (pbo_index + NUM_PBOS - 1) % NUM_PBOS;

            // 1️⃣ Issue async read into PBO
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[pbo_index]);
            glBindTexture(GL_TEXTURE_2D, fSim->mixedOut);
            glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            
            // 2️⃣ Map *previous* PBO (likely ready)
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[read]);
            void* ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            //printf("pointer value: %p\n",ptr);
            if (ptr) {
                conn.sendFrame((uint8_t*)ptr, G_WIDTH * G_HEIGHT * 3);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }

            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            pbo_index = next;
        }
    }
    printf("end\n");
    delete fSim;
    return 0;
}