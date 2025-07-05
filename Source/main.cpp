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
#include <random>

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

MethaneTexture LoadMethaneTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //rozmywa piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //œwietne dla pixel art

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    SDL_Surface* surf = IMG_Load(path);

    MethaneTexture metTex;
    metTex.texture = texture;
    if (!surf) {
        std::cout << "Failed to load image: " << IMG_GetError() << std::endl;
    }
    else {
        SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0); // Aby siê nie crashowa³o jak jest z³y format
        SDL_FreeSurface(surf);
        surf = formatted;
        surf = FlipSurfaceVertical(surf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels); // RGBA dla png
        metTex.w = surf->w;
        metTex.h = surf->h;
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    SDL_FreeSurface(surf);
    std::cout << "Textura za³adowana: " << metTex.texture << " " << metTex.w << " " << metTex.h << "\n";

    return metTex;
}

void PrintVec3(const glm::vec3 vec) {
    std::cout << vec.x << " " << vec.y << " " << vec.z << " \n";
}

glm::vec3 GenerateRandomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    return glm::vec3(dist(gen), dist(gen), dist(gen));
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

    

    MethaneTexture metTex1 = LoadMethaneTexture("Textures/testPNG.png");
    MethaneTexture metTex2 = LoadMethaneTexture("Textures/tree.png");


    //
    Renderer::Start(800,600);

    glm::vec3 color{ 1.0f,0.0f,0.0f };
    bool running = true;
    SDL_Event event;

    Rectangle rect{ 0,0,200,200 };
    Rectangle rect2{ 400,40,200,200 };
    Rectangle rect3{ 0,400,200,200 };
    RectangleF rectF{ 0.0f,0.0f,0.5f,0.5f };
    RectangleF rectF2{ -0.5f,-0.5f,0.5f,0.5f };
    Rectangle sourceRect{ 0,0,320,320 };
    RectangleF sourceRectF{ -0.5f,-0.5f,0.5f,0.5f };
    Rectangle rightUP{ 400,0,400,300 };
;

    float counter = 0;
    while (counter < 200 && running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        color = GenerateRandomColor();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < 1000; i++) {
            //Renderer::RenderCopyEX(rect, metTex1,counter);
            //Renderer::RenderCopyF(rectF, metTex2);
            //Renderer::RenderCopyFEX(rectF2, metTex1,90);
            //Renderer::RenderCopyPart(rect, sourceRect, metTex1);
            Renderer::RenderCopyPartEX(rect2, sourceRect, metTex1,90);
            Renderer::RenderCopyPartFEX(rectF2, sourceRectF, metTex1, counter);
            //Renderer::RenderRectangleFEX(sourceRectF, color, counter);
            

        }
        Renderer::RenderPresent();


        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << "Took: " << duration << " microseconds\n";

        
        
        counter++;
        SDL_GL_SwapWindow(window);
    }

    SDL_Delay(100000);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}