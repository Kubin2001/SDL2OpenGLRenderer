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



    glm::vec3 color{ 1.0f,0.0f,0.0f };
    bool running = true;
    SDL_Event event;

    MT::Rect rect{ 0,0,512,512 };
    MT::Rect sourceRect{125,125,125,125 };
    MT::Rect rect2{ 200,400,100,100 };
    MT::Rect rect3{ 0,400,200,200 };

    MT::RectF rectF2{ -0.5f,-0.5f,0.5f,0.5f };

    MT::RectF rectF{ -1.0f,1.0f,1.0f,1.0f };
    MT::RectF sourceRectF{ 0.122f,0.122f,0.122f,0.122f };

    MT::Rect rightUP{ 400,0,400,300 };

    MT::Color col1(255, 255, 255);
    MT::Color col2(0, 255, 0);

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


        //ren.RenderCopyPart(rect,sourceRect,*metTex1);
        //ren.RenderCopyPartF(rectF, sourceRectF, *metTex1);

        ren.RenderCopyPartEX(rect, sourceRect, *metTex1,counter);
        //ren.RenderCopyPartFEX(rectF, sourceRectF, *metTex1,0);
        //ren.RenderRect(rect3, col1);
        //ren.RenderRectEX(rect3, col2, counter);

        //ren.RenderCopy(rect2, *metTex2);
        //ren.RenderCopy(rect, *metTex1);

        //ren.RenderCircle(rect, col1, 0.55f);
        //ren.RenderCopyCircle(rect2, *metTex1,0.55f);
            
        rect2.w++;
        ren.RenderPresent();


        
        
        counter++;
        SDL_GL_SwapWindow(window);
    }

    SDL_Delay(100);
    ren.Clear();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}