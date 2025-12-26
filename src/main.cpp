#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <gl/glew.h>
#include <SDL3/SDL_opengl.h>
#include <gl/GLU.h>
#include <fstream>
#include "shader.hpp"
#include <windows.h>


void APIENTRY debugCallback(
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

/* Global Variables */
//The window we'll be rendering to
SDL_Window* gWindow = nullptr;
    
//The surface contained by the window
SDL_Surface* gScreenSurface = nullptr;

//The image we will load and show on the screen
SDL_Surface* gHelloWorld = nullptr;

SDL_GLContext gContext;

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

void printProgramLog( GLuint program )
{
    //Make sure name is shader
    if( glIsProgram( program ) )
    {
        //Program log length
        int infoLogLength = 0;
        int maxLength = infoLogLength;
        
        //Get info string length
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
        
        //Allocate string
        char* infoLog = new char[ maxLength ];
        
        //Get info log
        glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
        if( infoLogLength > 0 )
        {
            //Print Log
            printf( "%s\n", infoLog );
        }
        
        //Deallocate string
        delete[] infoLog;
    }
    else
    {
        printf( "Name %d is not a program\n", program );
    }
}

bool initGL() {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallback, NULL);
    //Success flag
    bool success = true;
    
    //Generate program
    gProgram.init();

    //initialize shaders
    if (!vertexShader.init("src/vertex.glsl")) {
        printf("Unable to compile vertex shader %d!\n", vertexShader.id);
        printShaderLog(vertexShader.id);
        return false;
    }

    if (!fragmentShader.init("src/frag.glsl")) {
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
            -0.5f, -0.5f,
                0.5f, -0.5f,
                0.5f,  0.5f,
            -0.5f,  0.5f
        };

        //IBO data
        GLuint indexData[] = { 0, 1, 2, 2, 3, 0 };

        //create VAO
        glGenVertexArrays(1, &gVAO);
        glBindVertexArray(gVAO);

        //Create VBO
        glGenBuffers( 1, &gVBO );
        glBindBuffer( GL_ARRAY_BUFFER, gVBO );
        glBufferData( GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW );

        //Create IBO
        glGenBuffers( 1, &gIBO );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indexData, GL_STATIC_DRAW );
    }
    return success;
}

/* Function Implementations */
bool init() {
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) == false) {
        SDL_Log( "SDL could not initialize! SDL error: %s\n", SDL_GetError() );
        success = false;
    } else {
        //Create window
        if(gWindow = SDL_CreateWindow( "Hello World!", kScreenWidth, kScreenHeight, SDL_WINDOW_OPENGL); gWindow == nullptr)
        {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }

        //Use OpenGL 4.3 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        //Create context
        gContext = SDL_GL_CreateContext(gWindow);
        if(gContext == NULL) {
            printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else {
            //Initialize GLEW
            glewExperimental = GL_TRUE; 
            GLenum glewError = glewInit();
            if(glewError != GLEW_OK) {
                printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
            }

            //Use Vsync
            if(!SDL_GL_SetSwapInterval(1)) {
                printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
            }

            //Initialize OpenGL
            if(!initGL()) {
                printf("Unable to initialize OpenGL!\n");
                success = false;
            }
        }
    }
    return success;
}

void render()
{
    //Clear color buffer
    glClear( GL_COLOR_BUFFER_BIT );
    
    //Render quad
    if( gRenderQuad )
    {
        //Bind program
        glUseProgram(gProgram.id);

        //Enable VAO
        glBindVertexArray(gVAO);

        //Enable vertex position
        glEnableVertexAttribArray(gVertexPos2DLocation);

        //Set vertex data
        glBindBuffer( GL_ARRAY_BUFFER, gVBO );
        glVertexAttribPointer( gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL );
        //Set index data and render
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL );

        //Disable vertex position
        glDisableVertexAttribArray( gVertexPos2DLocation );

        //Unbind program
        glUseProgram( NULL );
    }
}

void close()
{
    //Clean up surface
    SDL_DestroySurface(gHelloWorld);
    gHelloWorld = nullptr;
    
    //Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gScreenSurface = nullptr;

    //Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char** argv) {
    //Final exit code
    int exitCode = 0;
    //Initialize
    if(init() == false) {
        SDL_Log( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else
    {
        //The quit flag
        bool quit = false;

        //The event data
        SDL_Event e;
        SDL_zero(e);
        //The main loop
        while (quit == false) {
            //Get event data
            while(SDL_PollEvent(&e) == true) {
                //If event is quit type
                if(e.type == SDL_EVENT_QUIT) {
                    //End the main loop
                    quit = true;
                }
            }
            //Update the surface
            render();
            SDL_GL_SwapWindow(gWindow);
        }
    }
    return 0;
}