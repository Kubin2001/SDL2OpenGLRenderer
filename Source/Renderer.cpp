#include "Renderer.h"
#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"
#include <iostream>

unsigned int Renderer::VAO = 0;
unsigned int Renderer::VBO = 0;
unsigned int Renderer::EBO = 0;
unsigned int Renderer::W = 100;
unsigned int Renderer::H = 100;
unsigned int Renderer::currentProgram = 0;
unsigned int Renderer::textureLocation = 0;
unsigned int Renderer::RenderCopyExTransform;
unsigned int Renderer::renderCopyId = 0;
unsigned int Renderer::renderCopyExId = 0;
unsigned int Renderer::renderRectId = 0;
unsigned int Renderer::renderRectExId = 0;
unsigned int Renderer::currentTexture = -1;
unsigned int Renderer::renderRectMatrixLoc = -1;
unsigned int Renderer::alphaLoc = 0;

glm::mat4 Renderer::tarnsMatrix = glm::mat4(1.0f);

std::vector<float> Renderer::globalVertices = {};

unsigned int Renderer::indecies[6] = { 0, 1, 2, //Indeksy 1 trójkąta
                                      0, 2, 3 }; // indeksy 2 trójkąta
glm::vec2 RotateAndTranslate2D(float localX, float localY, const glm::vec2& center, float cosA, float sinA) {
    return {
        center.x + localX * cosA - localY * sinA,
        center.y + localX * sinA + localY * cosA
    };
}

bool Renderer::Start(unsigned int W, unsigned int H) {
    Renderer::W = W;
    Renderer::H = H;
    // Deklaracja zmiennych dla Vertex Array Object (VAO) i Vertex Buffer Object (VBO)
    // Generowanie VAO (Vertex Array Object) - obiekt przechowujący konfigurację atrybutów wierzchołków
    glGenVertexArrays(1, &VAO);

    // Generowanie VBO (Vertex Buffer Object) - bufor przechowujący dane wierzchołków
    glGenBuffers(1, &VBO);

    //Generowanie ebo aby nie musieć używać 6 wieżchołków
    glGenBuffers(1, &EBO);

    // Bindowanie VAO - od tego momentu wszystkie operacje na VAO będą dotyczyć tego obiektu
    glBindVertexArray(VAO);

    // Bindowanie VBO - od tego momentu wszystkie operacje na VBO będą dotyczyć tego bufora
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //Ustawnianie ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecies), indecies, GL_STATIC_DRAW);

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


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);




    // Przyśpieszacze
    ShaderLoader::LoadShader("VertexShader", "shaders/vertex_core.glsl", GL_VERTEX_SHADER);
    ShaderLoader::LoadShader("FragmentShader", "shaders/fragment_core.glsl", GL_FRAGMENT_SHADER);

    ShaderLoader::LoadShader("VertexShaderEX", "shaders/vertex_core_ex.glsl", GL_VERTEX_SHADER);
    ShaderLoader::LoadShader("FragmentShaderEX", "shaders/fragment_core_ex.glsl", GL_FRAGMENT_SHADER);

    ShaderLoader::LoadShader("VertexShaderRenderCopy", "shaders/vertex_core_render_copy.glsl", GL_VERTEX_SHADER);
    ShaderLoader::LoadShader("FragmentShaderRenderCopy", "shaders/fragment_core_render_copy.glsl", GL_FRAGMENT_SHADER);

    ShaderLoader::LoadShader("VertexShaderRenderCopyEX", "shaders/vertex_render_copy_ex.glsl", GL_VERTEX_SHADER);
    ShaderLoader::LoadShader("FragmentShaderRenderCopyEX", "shaders/fragment_render_copy_ex.glsl", GL_FRAGMENT_SHADER);



    std::vector<std::string> names = { "VertexShader" ,"FragmentShader" };
    ShaderLoader::CreateShaderProgram(names, "ShaderProgram");

    names = { "VertexShaderEX" ,"FragmentShaderEX" };
    ShaderLoader::CreateShaderProgram(names, "ShaderProgramEX");

    names = { "VertexShaderRenderCopy" ,"FragmentShaderRenderCopy" };
    ShaderLoader::CreateShaderProgram(names, "ShaderProgramRenderCopy");

    names = { "VertexShaderRenderCopyEX" ,"FragmentShaderRenderCopyEX" };
    ShaderLoader::CreateShaderProgram(names, "ShaderProgramRenderCopyEX");


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Renderer::renderCopyId = ShaderLoader::GetProgram("ShaderProgramRenderCopy");
    Renderer::renderRectId = ShaderLoader::GetProgram("ShaderProgram");
    Renderer::renderRectExId = ShaderLoader::GetProgram("ShaderProgramEX");
    Renderer::renderCopyExId = ShaderLoader::GetProgram("ShaderProgramRenderCopyEX");
    Renderer::renderRectMatrixLoc = glGetUniformLocation(ShaderLoader::GetProgram("ShaderProgramEX"), "transform");
    
    Renderer::textureLocation = glGetUniformLocation(Renderer::renderCopyId, "texture1");
    Renderer::RenderCopyExTransform = glGetUniformLocation(Renderer::renderCopyExId, "transform");

    Renderer::alphaLoc = glGetUniformLocation(Renderer::renderCopyId, "alpha");
    //globalVertices.reserve(1'000'000);
    return true;
}

void Renderer::ClearFrame(const unsigned char R, const unsigned char G, const unsigned char B) {
    const float fR = float(R) / 255;
    const float fG = float(G) / 255;
    const float fB = float(B) / 255;
    glClearColor(fR, fG, fB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::RenderRectF(const RectangleF& rect, const MethaneColor& col) {
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

void Renderer::RenderRect(const Rectangle& rect, const MethaneColor& col) {
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

void Renderer::RenderRectFEX(const RectangleF& rect, const MethaneColor &col, const float rotation) {
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


void Renderer::RenderRectEX(const Rectangle& rect, const MethaneColor &col, const float rotation) {
    // Włączenie atrybututów iwerzchołków
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    RectangleF temp;
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



void Renderer::RenderCopyF(const RectangleF& rect, const MethaneTexture& texture) {
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

void Renderer::RenderCopy(const Rectangle& rect, const MethaneTexture& texture){
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

void Renderer::RenderCopyPartF(const RectangleF& rect, const RectangleF& source, const MethaneTexture& texture) {
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
    const float v0 = source.y;
    const float u1 = source.x + source.w;
    const float v1 = source.y + source.h;

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

void Renderer::RenderCopyPart(const Rectangle& rect, const Rectangle& source, const MethaneTexture &texture) {
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

    RectangleF tempSource;

    tempSource.x = static_cast<float>(source.x) / texture.w;
    tempSource.y = static_cast<float>(source.y) / texture.h;
    tempSource.w = static_cast<float>(source.w) / texture.w;
    tempSource.h = static_cast<float>(source.h) / texture.h;

    float u0 = tempSource.x;
    float v0 = tempSource.y;
    float u1 = tempSource.x + tempSource.w;
    float v1 = tempSource.y + tempSource.h;

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
}

void Renderer::RenderCopyFEX(const RectangleF& rect, const MethaneTexture& texture, const float rotation) {
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



void Renderer::RenderCopyEX(const Rectangle& rect, const MethaneTexture& texture, const float rotation) {
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

void Renderer::RenderCopyPartFEX(const RectangleF& rect, const RectangleF& source, const MethaneTexture& texture, const float rotation) {
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
    const float v0 = source.y;
    const float u1 = source.x + source.w;
    const float v1 = source.y + source.h;

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

void Renderer::RenderCopyPartEX(const Rectangle& rect, const Rectangle& source, const MethaneTexture& texture, const float rotation) {
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
    const float v0 = static_cast<float>(source.y) / texH;
    const float u1 = static_cast<float>(source.x + source.w) / texW;
    const float v1 = static_cast<float>(source.y + source.h) / texH;

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


void Renderer::RenderPresent() {
    if (globalVertices.empty()) {
        return;
    }  

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, globalVertices.size() * sizeof(float), globalVertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, globalVertices.size());

    globalVertices.clear();
}

void Renderer::Clear() {
    // Odwiązanie VAO - bezpieczne praktyka, aby nie modyfikować przypadkowo tego VAO w przyszłości
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // czyszczenie aby nie było wycieków pamięci nie tworzyć jak vbo i vao są globalnie zadeklarowane
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteBuffers(1, &EBO);
}