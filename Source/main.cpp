#include <SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include "Renderer.h"
#include "ShaderLoader.h"
#include <SDL_image.h>
#include <chrono>



void PrintVec3(const glm::vec3 vec) {
    std::cout << vec.x << " " << vec.y << " " << vec.z << " \n";
}

SDL_Surface* FlipSurfaceVertical(SDL_Surface* surface) {
    SDL_Surface* flipped = SDL_CreateRGBSurfaceWithFormat(0, surface->w, surface->h,
        surface->format->BitsPerPixel,
        surface->format->format);
    int pitch = surface->pitch;
    uint8_t* srcPixels = (uint8_t*)surface->pixels;
    uint8_t* dstPixels = (uint8_t*)flipped->pixels;

    for (int y = 0; y < surface->h; ++y) {
        memcpy(&dstPixels[y * pitch],
            &srcPixels[(surface->h - 1 - y) * pitch],
            pitch);
    }

    return flipped;
}


int main(int argc, char* argv[]) {

    int success;
    char infoLog[512];

    SDL_Init(SDL_INIT_VIDEO);
    // Set OpenGL version and profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);


    SDL_GLContext glContext = SDL_GL_CreateContext(window);


    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //rozmywa piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //œwietne dla pixel art

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    SDL_Surface* surf = IMG_Load("Textures/testPNG2.png");

    MethaneTexture metTex;
    metTex.texture = texture;
    if (!surf) {
        std::cout << "Failed to load image: " << IMG_GetError() << std::endl;
    }
    else {

        SDL_Surface* flipped = FlipSurfaceVertical(surf);
        surf = flipped;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels); // RGBA dla png
        metTex.w = surf->w;
        metTex.h = surf->h;
        glGenerateMipmap(GL_TEXTURE_2D);

        SDL_FreeSurface(surf);
    }

    std::cout << "Textura: " << metTex.texture << " " << metTex.w << " " << metTex.h << "\n";

    




    //
    Renderer::Start(800,600);

    glm::vec3 color{ 1.0f,0.0f,0.0f };
    bool running = true;
    SDL_Event event;

    Rectangle rect{ 0,0,100,100 };


    float counter = 0;
    while (counter < 1000) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        // Start pomiaru czasu
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 1000; ++i) {
            Renderer::RenderCopy(rect, metTex);
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto totalDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        auto avgDuration = totalDuration / 1000;

        std::cout << "Œredni czas RenderCopy: " << avgDuration << " ns" << std::endl;

        
        counter++;
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}