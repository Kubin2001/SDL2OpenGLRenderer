#include "Renderer.h"
#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
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

SDL_GLContext MT::Innit(SDL_Window *window) {

    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    
    return context;
}

MT::Texture* MT::LoadTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //rozmywa piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //świetne dla pixel art

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    SDL_Surface* surf = IMG_Load(path);

    MT::Texture *metTex = new MT::Texture;
    metTex->texture = texture;
    if (!surf) {
        std::cout << "Failed to load image MT::LoadTexture: " << IMG_GetError() << "\n";
        return metTex;
    }
    else {
        SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0); // Aby się nie crashowało jak jest zły format
        SDL_FreeSurface(surf);
        surf = formatted;
        surf = FlipSurfaceVertical(surf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels); // RGBA dla png
        metTex->w = surf->w;
        metTex->h = surf->h;
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    SDL_FreeSurface(surf);

    return metTex;
}

MT::Texture* MT::LoadTextureFromSurface(SDL_Surface* surf) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //rozmywa piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //świetne dla pixel art

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    MT::Texture* metTex = new MT::Texture;
    metTex->texture = texture;
    if (!surf) {
        std::cout << "Empty surface in MT::LoadTextureFromSurface: " << IMG_GetError() << std::endl;
        return metTex;
    }

    SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0); // Aby się nie crashowało jak jest zły format
    SDL_Surface* flipped = FlipSurfaceVertical(formatted);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, flipped->w, flipped->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, flipped->pixels); // RGBA dla png
    metTex->w = flipped->w;
    metTex->h = flipped->h;
    glGenerateMipmap(GL_TEXTURE_2D);
    SDL_FreeSurface(flipped);
    SDL_FreeSurface(formatted);

    return metTex;
}

glm::vec2 RotateAndTranslate2D(float localX, float localY, const glm::vec2& center, float cosA, float sinA) {
    return {
        center.x + localX * cosA - localY * sinA,
        center.y + localX * sinA + localY * cosA
    };
}

bool MT::Renderer::Start(SDL_Window* window, SDL_GLContext context) {

    SDL_GL_GetDrawableSize(window, &W, &H);
    this->context = context;
    // Deklaracja zmiennych dla Vertex Array Object (VAO) i Vertex Buffer Object (VBO)
    // Generowanie VAO (Vertex Array Object) - obiekt przechowujący konfigurację atrybutów wierzchołków
    glGenVertexArrays(1, &VAO);

    // Generowanie VBO (Vertex Buffer Object) - bufor przechowujący dane wierzchołków
    glGenBuffers(1, &VBO);

    // Bindowanie VAO - od tego momentu wszystkie operacje na VAO będą dotyczyć tego obiektu
    glBindVertexArray(VAO);

    // Bindowanie VBO - od tego momentu wszystkie operacje na VBO będą dotyczyć tego bufora
    glBindBuffer(GL_ARRAY_BUFFER, VBO);


    // Konfiguracja atrybutu wierzchołka - mówi OpenGL, jak interpretować dane w buforze
    // 0 - indeks atrybutu (w shaderze odpowiada location = 0)
    // 3 - liczba składowych (x, y, z)
    // GL_FLOAT - typ danych
    // GL_FALSE - czy normalizować dane (nie w tym przypadku)
    // 3 * sizeof(float) - odległość między kolejnymi wierzchołkami (w bajtach)
    // (void*)0 - przesunięcie do pierwszego elementu w buforze

    //Gdy nie ma tekstur
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // powierzchnie
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))); // kolory

    //Gdy są tekstury
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // powierzchnie
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // tekstury

    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // powierzchnie
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // kolory
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // uv dla koła


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);

    if (!ShaderLoader::IsProgram("ShaderProgram")) {
        const std::string vertexShaderStr = R"glsl(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;

            out vec3 ourColor;

            void main() {
                gl_Position = vec4(aPos, 1.0);
                ourColor = aColor;
            }
        )glsl";

            const std::string fragmentShaderStr = R"glsl(
        #version 330 core

        out vec4 FragColor;

        in vec3 ourColor;

        void main(){
	        FragColor = vec4(ourColor,1.0);
        }
        )glsl";


        ShaderLoader::LoadShaderStr("VertexShader", vertexShaderStr, GL_VERTEX_SHADER);
        ShaderLoader::LoadShaderStr("FragmentShader", fragmentShaderStr, GL_FRAGMENT_SHADER);

        std::vector<std::string> names = { "VertexShader" ,"FragmentShader" };
        ShaderLoader::CreateShaderProgram(names, "ShaderProgram");
    }

    if (!ShaderLoader::IsProgram("ShaderProgramRecAlpha")) {
        const std::string vertexShaderStr = R"glsl(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;

            out vec3 ourColor;

            void main() {
                gl_Position = vec4(aPos, 1.0);
                ourColor = aColor;
            }
        )glsl";

        const std::string fragmentShaderStr = R"glsl(
        #version 330 core

        out vec4 FragColor;

        in vec3 ourColor;

        uniform float alpha;

        void main(){
	        FragColor = vec4(ourColor,1.0 *alpha);
        }
        )glsl";


        ShaderLoader::LoadShaderStr("VertexShaderAlpha", vertexShaderStr, GL_VERTEX_SHADER);
        ShaderLoader::LoadShaderStr("FragmentShaderAlpha", fragmentShaderStr, GL_FRAGMENT_SHADER);

        std::vector<std::string> names = { "VertexShaderAlpha" ,"FragmentShaderAlpha" };
        ShaderLoader::CreateShaderProgram(names, "ShaderProgramAlpha");
    }

    if (!ShaderLoader::IsProgram("ShaderProgramRenderCopy")) {
        const std::string vertexRenderCopyStr = R"glsl(
        #version 330 core
        layout (location = 2) in vec3 aPos;
        layout (location = 3) in vec2 aTexCord;

        out vec3 ourColor;
        out vec2 texCord;

        void main(){
	        gl_Position = vec4(aPos ,1.0);

	        texCord = aTexCord;
        }
        )glsl";

            const std::string fragmentRenderCopyStr = R"glsl(
        #version 330 core

        out vec4 FragColor;

        in vec2 texCord;

        uniform sampler2D texture1;

        uniform float alpha;

        void main(){
	        vec4 texcolor = texture(texture1,texCord);
	        texcolor.a *= alpha;
	        FragColor = texcolor;
        }
        )glsl";



        ShaderLoader::LoadShaderStr("VertexShaderRenderCopy", vertexRenderCopyStr, GL_VERTEX_SHADER);
        ShaderLoader::LoadShaderStr("FragmentShaderRenderCopy", fragmentRenderCopyStr, GL_FRAGMENT_SHADER);



        std::vector<std::string> names2 = { "VertexShaderRenderCopy" ,"FragmentShaderRenderCopy" };
        ShaderLoader::CreateShaderProgram(names2, "ShaderProgramRenderCopy");
    }

    if (!ShaderLoader::IsProgram("ShaderProgramRenderCopyCircle")) {
        const std::string vertexRenderCopyCircleStr = R"glsl(
        #version 330 core
        layout (location = 2) in vec3 aPos;
        layout (location = 3) in vec2 aTexCord;

        out vec2 texCord;

        void main(){
	        gl_Position = vec4(aPos ,1.0);

	        texCord = aTexCord;
        }
        )glsl";

        const std::string fragementRenderCopyCircleStr = R"glsl(
        #version 330 core
        in vec2 texCord;
        out vec4 FragColor;

        uniform sampler2D texture0;
        uniform float alpha;
        uniform float radius;

        void main()
        {
            vec2 center = vec2(0.5, 0.5);

            float dist = distance(texCord, center);
            if (dist > radius)
                discard;

            vec4 texColor = texture(texture0, texCord);
            FragColor = vec4(texColor.rgb, texColor.a * alpha);
        }
        )glsl";



        ShaderLoader::LoadShaderStr("VertexShaderRenderCopyCircle", vertexRenderCopyCircleStr, GL_VERTEX_SHADER);
        ShaderLoader::LoadShaderStr("FragmentShaderRenderCopyCircle", fragementRenderCopyCircleStr, GL_FRAGMENT_SHADER);


        std::vector<std::string> names2 = { "VertexShaderRenderCopyCircle" ,"FragmentShaderRenderCopyCircle" };
        ShaderLoader::CreateShaderProgram(names2, "ShaderProgramRenderCopyCircle");
    }

    if (!ShaderLoader::IsProgram("ShaderProgramRenderCircle")) {
        const std::string vertexRenderCircleStr = R"glsl(
        #version 330 core
        layout(location = 4) in vec3 aPos;
        layout(location = 5) in vec3 aColor;
        layout(location = 6) in vec2 aUv;

        out vec3 ourColor;
        out vec2 uv;

        void main() {
            gl_Position = vec4(aPos, 1.0);
            ourColor = aColor;
            uv = aUv;
        }
        )glsl";

        const std::string fragementRenderCircleStr = R"glsl(
        #version 330 core

        in vec3 ourColor;
        in vec2 uv;
        out vec4 FragColor;

        uniform float radius;

        void main(){
            vec2 center = vec2(0.5, 0.5);
            float dist = distance(uv, center);
            if (dist > radius)
                discard;

            FragColor = vec4(ourColor, 1.0);
        }
        )glsl";



        ShaderLoader::LoadShaderStr("VertexShaderRenderCircle", vertexRenderCircleStr, GL_VERTEX_SHADER);
        ShaderLoader::LoadShaderStr("FragmentShaderRenderCircle", fragementRenderCircleStr, GL_FRAGMENT_SHADER);


        std::vector<std::string> names = { "VertexShaderRenderCircle" ,"FragmentShaderRenderCircle" };
        ShaderLoader::CreateShaderProgram(names, "ShaderProgramRenderCircle");
    }


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderCopyId = ShaderLoader::GetProgram("ShaderProgramRenderCopy");
    renderRectId = ShaderLoader::GetProgram("ShaderProgram");
    renderCopyCircleId = ShaderLoader::GetProgram("ShaderProgramRenderCopyCircle");
    renderCircleId = ShaderLoader::GetProgram("ShaderProgramRenderCircle");
    renderRectAlphaId = ShaderLoader::GetProgram("ShaderProgramAlpha");
    

    textureLocation = glGetUniformLocation(Renderer::renderCopyId, "texture1");


    alphaLoc = glGetUniformLocation(renderCopyId, "alpha");
    alphaLocRect = glGetUniformLocation(renderRectAlphaId, "alpha");
    radiusLoc = glGetUniformLocation(renderCopyCircleId, "radius");
    radiusLoc2 = glGetUniformLocation(renderCircleId, "radius");
    currentRadius = 0.5f;
    glUniform1f(radiusLoc, currentRadius);
    glUniform1f(radiusLoc2, currentRadius);
    //globalVertices.reserve(1'000'000);
    return true;
}

void MT::Renderer::ClearFrame(const unsigned char R, const unsigned char G, const unsigned char B) {
    const float fR = float(R) / 255;
    const float fG = float(G) / 255;
    const float fB = float(B) / 255;
    glClearColor(fR, fG, fB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void MT::Renderer::RenderRectF(const RectF& rect, const Color& col) {
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;

    float vertices[] = {
        rect.x,     rect.y - rect.h,      0.0f,    fR, fG, fB,
        rect.x,     rect.y,               0.0f,    fR, fG, fB,
        rect.x + rect.w, rect.y - rect.h, 0.0f,    fR, fG, fB,
        rect.x,     rect.y,               0.0f,    fR, fG, fB,
        rect.x + rect.w, rect.y,          0.0f,    fR, fG, fB,
        rect.x + rect.w, rect.y - rect.h, 0.0f,    fR, fG, fB,
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertices), std::end(vertices));
}

void MT::Renderer::RenderRect(const Rect& rect, const Color& col) {
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    float w = (static_cast<float>(rect.w) / W) * 2.0f;
    float h = (static_cast<float>(rect.h) / H) * 2.0f;

    // Wersja gdzie punkt 0.0 jest w lewym dolnym
    // -1.0f dlatego że nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego że w innym wypadku ekran byłby traktowany jakby miał powójną szerokość
    //temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    //temp.y = (static_cast<float>(rect.y) / H) * 2.0f - 1.0f;
    //temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    //temp.h = (static_cast<float>(rect.h) / H) * 2.0f;
    //std::cout <<temp.x<<"  " << temp.y << "\n";

    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;


    // pos.x, pos.y, pos.z,  col.r, col.g, col.b
    float vertices[] = {
        x,     y - h, 0.0f, fR, fG, fB,
        x,     y,     0.0f, fR, fG, fB,
        x + w, y - h, 0.0f, fR, fG, fB,
        x,     y,     0.0f, fR, fG, fB,
        x + w, y,     0.0f, fR, fG, fB,
        x + w, y - h, 0.0f, fR, fG, fB,
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertices), std::end(vertices));
}

void MT::Renderer::RenderRectFEX(const RectF& rect, const Color &col, const float rotation) {
    // Włączenie atrybututów iwerzchołków
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }
    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;

    float halfW = rect.w / 2.0f;
    float halfH = rect.h / 2.0f;

    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { rect.x + halfW, rect.y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, fR, fG, fB,
        p1.x, p1.y, 0.0f, fR, fG, fB,
        p2.x, p2.y, 0.0f, fR, fG, fB,
        p3.x, p3.y, 0.0f, fR, fG, fB,
        p4.x, p4.y, 0.0f, fR, fG, fB,
        p5.x, p5.y, 0.0f, fR, fG, fB,
    };
    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}


void MT::Renderer::RenderRectEX(const Rect& rect, const Color &col, const float rotation) {
    // Włączenie atrybututów iwerzchołków
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    RectF temp;
    // -1.0f dlatego że nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego że w innym wypadku ekran byłby traktowany jakby miał powójną szerokość
    float aspect = static_cast<float>(H) / static_cast<float>(W);
    temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    temp.y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    temp.h = (static_cast<float>(rect.h) / H) * 2.0f * aspect;

    // Wersja gdzie punkt 0.0 jest w lewym dolnym
    // -1.0f dlatego że nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego że w innym wypadku ekran byłby traktowany jakby miał powójną szerokość
    //temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    //temp.y = (static_cast<float>(rect.y) / H) * 2.0f - 1.0f;
    //temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    //temp.h = (static_cast<float>(rect.h) / H) * 2.0f;
    //std::cout <<temp.x<<"  " << temp.y << "\n";

    // Wierzchołki zdefiniowane względem środka prostokąta
    // Wierzchołki zdefiniowane względem środka prostokąta

    float halfW = temp.w / 2.0f;
    float halfH = temp.h / 2.0f;

    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;

    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { temp.x + halfW, temp.y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, fR, fG, fB,
        p1.x, p1.y, 0.0f, fR, fG, fB,
        p2.x, p2.y, 0.0f, fR, fG, fB,
        p3.x, p3.y, 0.0f, fR, fG, fB,
        p4.x, p4.y, 0.0f, fR, fG, fB,
        p5.x, p5.y, 0.0f, fR, fG, fB,
    };
    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}



void MT::Renderer::RenderCopyF(const RectF& rect, const Texture& texture) {
    // Włączenie atrybututów iwerzchołków
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }


    // Przypisanie tekstury do samplera (uniforma 'texture1') - tekstura 0
    //glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, że przypisujemy teksturę do GL_TEXTURE0


    float verticles[30] = {
        rect.x,          rect.y - rect.h, 0.0f, 0.0f, 0.0f,
        rect.x,          rect.y,          0.0f, 0.0f, 1.0f,
        rect.x + rect.w, rect.y - rect.h, 0.0f, 1.0f, 0.0f,
        rect.x,          rect.y,          0.0f, 0.0f, 1.0f,
        rect.x + rect.w, rect.y,          0.0f, 1.0f, 1.0f,
        rect.x + rect.w, rect.y - rect.h, 0.0f, 1.0f, 0.0f
    };
    globalVertices.insert(globalVertices.end(), std::begin(verticles), std::end(verticles));

}

void MT::Renderer::RenderCopy(const Rect& rect, const Texture& texture){
    const float x = (rect.x / static_cast<float>(W)) * 2.0f - 1.0f;
    const float y = 1.0f - (rect.y / static_cast<float>(H)) * 2.0f;
    const float w = (rect.w / static_cast<float>(W)) * 2.0f;
    const float h = (rect.h / static_cast<float>(H)) * 2.0f;

    // aktywacja tekstury
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }
    
    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }

    glUniform1f(alphaLoc, texture.alpha);
    

    //    // pos.x, pos.y, pos.z, tex.u, tex.v
    float verticles[30] = {
        x,     y - h, 0.0f, 0.0f, 0.0f,
        x,     y,     0.0f, 0.0f, 1.0f,
        x + w, y - h, 0.0f, 1.0f, 0.0f,
        x,     y,     0.0f, 0.0f, 1.0f,
        x + w, y,     0.0f, 1.0f, 1.0f,
        x + w, y - h, 0.0f, 1.0f, 0.0f
    };
    globalVertices.insert(globalVertices.end(), std::begin(verticles), std::end(verticles));
}

void MT::Renderer::RenderCopyPartF(const RectF& rect, const RectF& source, const Texture& texture) {
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    glUniform1f(alphaLoc, texture.alpha);


    const float u0 = source.x;
    const float u1 = source.x + source.w;
    const float v1 = 1.0f - source.y;
    const float v0 = v1 - source.h;


    float verticles[30] = {
        rect.x,          rect.y - rect.h, 0.0f, u0, v0,
        rect.x,          rect.y,          0.0f, u0, v1,
        rect.x + rect.w, rect.y - rect.h, 0.0f, u1, v0,
        rect.x,          rect.y,          0.0f, u0, v1,
        rect.x + rect.w, rect.y,          0.0f, u1, v1,
        rect.x + rect.w, rect.y - rect.h, 0.0f, u1, v0
    };

    globalVertices.insert(globalVertices.end(), std::begin(verticles), std::end(verticles));
}

void MT::Renderer::RenderCopyPart(const Rect& rect, const Rect& source, const Texture &texture) {
    const float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    const float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    const float w = (static_cast<float>(rect.w) / W) * 2.0f;
    const float h = (static_cast<float>(rect.h) / H) * 2.0f;


    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    glUniform1f(alphaLoc, texture.alpha);

    RectF tempSource;

    tempSource.x = static_cast<float>(source.x) / texture.w;
    tempSource.y = static_cast<float>(source.y) / texture.h;
    tempSource.w = static_cast<float>(source.w) / texture.w;
    tempSource.h = static_cast<float>(source.h) / texture.h;

    float u0 = tempSource.x;
    float u1 = tempSource.x + tempSource.w;
    float v1 = 1.0f - tempSource.y;
    float v0 = v1 - tempSource.h;


    // pos.x pos.y pos.z, tex.u, tex.v
    float verticles[30] = {
        x,     y - h, 0.0f, u0, v0,
        x,     y,     0.0f, u0, v1,
        x + w, y - h, 0.0f, u1, v0,
        x,     y,     0.0f, u0, v1,
        x + w, y,     0.0f, u1, v1,
        x + w, y - h, 0.0f, u1, v0
    };
    globalVertices.insert(globalVertices.end(), std::begin(verticles), std::end(verticles));
    //printf("u0=%.3f, v0=%.3f, u1=%.3f, v1=%.3f\n", u0, v0, u1, v1);
}

void MT::Renderer::RenderCopyFEX(const RectF& rect, const Texture& texture, const float rotation) {
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    glUniform1f(alphaLoc, texture.alpha);

    float halfW = rect.w / 2.0f;
    float halfH = rect.h / 2.0f;

    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { rect.x + halfW, rect.y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, 0.0f, 0.0f,
        p1.x, p1.y, 0.0f, 0.0f, 1.0f,
        p2.x, p2.y, 0.0f, 1.0f, 1.0f,
        p3.x, p3.y, 0.0f, 0.0f, 0.0f,
        p4.x, p4.y, 0.0f, 1.0f, 1.0f,
        p5.x, p5.y, 0.0f, 1.0f, 0.0f
    };
    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}

void MT::Renderer::RenderCopyEX(const Rect& rect, const Texture& texture, const float rotation) {
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    glUniform1f(alphaLoc, texture.alpha);

    float aspect = static_cast<float>(H) / static_cast<float>(W);
    float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    float w = (static_cast<float>(rect.w) / W) * 2.0f;
    float h = (static_cast<float>(rect.h) / H) * 2.0f * aspect;

    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { x + halfW, y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, 0.0f, 0.0f,
        p1.x, p1.y, 0.0f, 0.0f, 1.0f,
        p2.x, p2.y, 0.0f, 1.0f, 1.0f,
        p3.x, p3.y, 0.0f, 0.0f, 0.0f,
        p4.x, p4.y, 0.0f, 1.0f, 1.0f,
        p5.x, p5.y, 0.0f, 1.0f, 0.0f
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}

void MT::Renderer::RenderCopyPartFEX(const RectF& rect, const RectF& source, const Texture& texture, const float rotation) {
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    glUniform1f(alphaLoc, texture.alpha);

    ////////
    float halfW = rect.w / 2.0f;
    float halfH = rect.h / 2.0f;

    const float u0 = source.x;
    const float u1 = source.x + source.w;
    const float v1 = 1.0f - source.y;
    const float v0 = v1 - source.h;

    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { rect.x + halfW, rect.y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, u0, v0,
        p1.x, p1.y, 0.0f, u0, v1,
        p2.x, p2.y, 0.0f, u1, v1,
        p3.x, p3.y, 0.0f, u0, v0,
        p4.x, p4.y, 0.0f, u1, v1,
        p5.x, p5.y, 0.0f, u1, v0
    };
    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}

void MT::Renderer::RenderCopyPartEX(const Rect& rect, const Rect& source, const Texture& texture, const float rotation) {
    if (Renderer::currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (Renderer::currentProgram != Renderer::renderCopyId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    glUniform1f(alphaLoc, texture.alpha);

    float aspect = static_cast<float>(H) / static_cast<float>(W);
    const float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    const float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    const float w = (static_cast<float>(rect.w) / W) * 2.0f;
    const float h = (static_cast<float>(rect.h) / H) * 2.0f *aspect;

    float halfW = w / 2.0f;
    float halfH = h / 2.0f;

    const float texW = static_cast<float>(texture.w);
    const float texH = static_cast<float>(texture.h);


    const float u0 = static_cast<float>(source.x) / texW;
    const float u1 = static_cast<float>(source.x + source.w) / texW;

    const float v1 = 1.0f - static_cast<float>(source.y) / texH;
    const float v0 = v1 - static_cast<float>(source.h) / texH;


    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { x + halfW, y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, u0, v0,
        p1.x, p1.y, 0.0f, u0, v1,
        p2.x, p2.y, 0.0f, u1, v1,
        p3.x, p3.y, 0.0f, u0, v0,
        p4.x, p4.y, 0.0f, u1, v1,
        p5.x, p5.y, 0.0f, u1, v0
    };
    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}


void MT::Renderer::RenderCopyCircle(const Rect& rect, const Texture& texture, const float radius) {
    const float x = (rect.x / static_cast<float>(W)) * 2.0f - 1.0f;
    const float y = 1.0f - (rect.y / static_cast<float>(H)) * 2.0f;
    const float w = (rect.w / static_cast<float>(W)) * 2.0f;
    const float h = (rect.h / static_cast<float>(H)) * 2.0f;

    // aktywacja tekstury


    if (currentTexture != texture.texture) {
        RenderPresent();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture.texture);
        currentTexture = texture.texture;
    }

    if (currentProgram != renderCopyCircleId) {
        RenderPresent();
        currentProgram = renderCopyCircleId;
        glUseProgram(renderCopyCircleId);
        glUniform1f(radiusLoc, currentRadius);
    }

    if (currentRadius != radius) {
        currentRadius = radius;
        RenderPresent();
        glUniform1f(radiusLoc, currentRadius);
    }

    glUniform1f(alphaLoc, texture.alpha);


    //    // pos.x, pos.y, pos.z, tex.u, tex.v
    float verticles[30] = {
        x,     y - h, 0.0f, 0.0f, 0.0f,
        x,     y,     0.0f, 0.0f, 1.0f,
        x + w, y - h, 0.0f, 1.0f, 0.0f,
        x,     y,     0.0f, 0.0f, 1.0f,
        x + w, y,     0.0f, 1.0f, 1.0f,
        x + w, y - h, 0.0f, 1.0f, 0.0f
    };
    globalVertices.insert(globalVertices.end(), std::begin(verticles), std::end(verticles));
}

void MT::Renderer::RenderCircle(const Rect& rect, const Color& col, const float radius) {
    if (currentProgram != renderCircleId) {
        RenderPresent();
        currentProgram = renderCircleId;
        glUseProgram(renderCircleId);
        glUniform1f(radiusLoc2, currentRadius);
    }

    if (currentRadius != radius) {
        currentRadius = radius;
        RenderPresent();
        glUniform1f(radiusLoc2, currentRadius);
    }

    float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    float w = (static_cast<float>(rect.w) / W) * 2.0f;
    float h = (static_cast<float>(rect.h) / H) * 2.0f;

    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;


    // pos.x, pos.y, pos.z,  col.r, col.g, col.b u,v
    float vertices[] = {
        x,     y - h, 0.0f,   fR, fG, fB,   0.0f, 0.0f,
        x,     y,     0.0f,   fR, fG, fB,   0.0f, 1.0f,
        x + w, y - h, 0.0f,   fR, fG, fB,   1.0f, 0.0f,
        x,     y,     0.0f,   fR, fG, fB,   0.0f, 1.0f,
        x + w, y,     0.0f,   fR, fG, fB,   1.0f, 1.0f,
        x + w, y - h, 0.0f,   fR, fG, fB,   1.0f, 0.0f
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertices), std::end(vertices));
}

void MT::Renderer::RenderRectAlpha(const Rect& rect, const Color& col, unsigned char alpha) {
    if (Renderer::currentProgram != renderRectAlphaId) {
        RenderPresent();
        Renderer::currentProgram = renderRectAlphaId;
        glUseProgram(renderRectAlphaId);
    }

    float floatAlpha = float(alpha) / 255;
    glUniform1f(alphaLoc, floatAlpha);

    float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    float w = (static_cast<float>(rect.w) / W) * 2.0f;
    float h = (static_cast<float>(rect.h) / H) * 2.0f;

    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;


    // pos.x, pos.y, pos.z,  col.r, col.g, col.b
    float vertices[] = {
        x,     y - h, 0.0f, fR, fG, fB,
        x,     y,     0.0f, fR, fG, fB,
        x + w, y - h, 0.0f, fR, fG, fB,
        x,     y,     0.0f, fR, fG, fB,
        x + w, y,     0.0f, fR, fG, fB,
        x + w, y - h, 0.0f, fR, fG, fB,
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertices), std::end(vertices));
}

void MT::Renderer::RenderRectAlphaEX(const Rect& rect, const Color& col, unsigned char alpha, const float rotation) {
    if (Renderer::currentProgram != renderRectAlphaId) {
        RenderPresent();
        Renderer::currentProgram = renderRectAlphaId;
        glUseProgram(renderRectAlphaId);
    }

    float floatAlpha = float(alpha) / 255;
    glUniform1f(alphaLoc, floatAlpha);

    RectF temp;
    float aspect = static_cast<float>(H) / static_cast<float>(W);
    temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    temp.y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    temp.h = (static_cast<float>(rect.h) / H) * 2.0f;

    float halfW = temp.w / 2.0f;
    float halfH = temp.h / 2.0f;

    const float fR = float(col.R) / 255;
    const float fG = float(col.G) / 255;
    const float fB = float(col.B) / 255;

    float rad = glm::radians(rotation);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec2 center = { temp.x + halfW, temp.y - halfH };

    glm::vec2 p0 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p1 = RotateAndTranslate2D(-halfW, halfH, center, cosA, sinA);
    glm::vec2 p2 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p3 = RotateAndTranslate2D(-halfW, -halfH, center, cosA, sinA);
    glm::vec2 p4 = RotateAndTranslate2D(halfW, halfH, center, cosA, sinA);
    glm::vec2 p5 = RotateAndTranslate2D(halfW, -halfH, center, cosA, sinA);

    const float vertex[] = {
        p0.x, p0.y, 0.0f, fR, fG, fB,
        p1.x, p1.y, 0.0f, fR, fG, fB,
        p2.x, p2.y, 0.0f, fR, fG, fB,
        p3.x, p3.y, 0.0f, fR, fG, fB,
        p4.x, p4.y, 0.0f, fR, fG, fB,
        p5.x, p5.y, 0.0f, fR, fG, fB,
    };
    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}


void MT::Renderer::RenderPresent() {
    if (globalVertices.empty()) {
        return;
    }  

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, globalVertices.size() * sizeof(float), globalVertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, globalVertices.size());

    globalVertices.clear();
}



void MT::Renderer::Clear() {
    // Odwiązanie VAO - bezpieczne praktyka, aby nie modyfikować przypadkowo tego VAO w przyszłości
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // czyszczenie aby nie było wycieków pamięci nie tworzyć jak vbo i vao są globalnie zadeklarowane
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    SDL_GL_DeleteContext(context);
}