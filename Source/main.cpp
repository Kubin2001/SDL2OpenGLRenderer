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

glm::vec3 GenerateRandomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    return glm::vec3(dist(gen), dist(gen), dist(gen));
}


int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window* window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); 

    MT::Renderer ren;
    ren.Start(window, MT::Innit(window));

    MT::Texture* metTex1 = MT::LoadTexture("Textures/testPNG2.png");
    MT::Texture* metTex2 = MT::LoadTexture("Textures/tree.png"); 
    MT::Texture* letterTex = MT::LoadTexture("Textures/TestLetter.png");



    glm::vec3 color{ 1.0f,0.0f,0.0f };
    bool running = true;
    SDL_Event event;

    MT::Rect rect{ 100,100,100,100 };
    MT::Rect rect2{ 400,100,100,100 };
    MT::Rect source{ 0,0,40,20 };


    metTex1->SetAlphaBending(180);

    float counter = 0;
    while (counter < 100000 && running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        color = GenerateRandomColor();
        ren.ClearFrame(40,40,40);


        //ren.RenderCopy(rect, *metTex1);
        //ren.RenderCopy(rect, *letterTex);
        ren.RenderCopyFiltered(rect2, *letterTex, {255,255,255});
        ren.RenderCopyPartFiltered(rect, source, *letterTex, { 0,255,255 });

        //ren.RenderCopy(rect2, *metTex1);
        //ren.RenderRectAlphaEX(rect2, { 255,255,255 }, 100,counter);
            
        ren.RenderPresent();
        counter++;
    }

    SDL_Delay(100);
    ren.Clear();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}