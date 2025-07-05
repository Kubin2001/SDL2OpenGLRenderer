#include "Renderer.h"
#include "glad/glad.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
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
glm::mat4 Renderer::tarnsMatrix = glm::mat4(1.0f);

std::vector<float> Renderer::globalVertices = {};

unsigned int Renderer::indecies[6] = { 0, 1, 2, //Indeksy 1 trójkąta
                                      0, 2, 3 }; // indeksy 2 trójkąta


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
    //globalVertices.reserve(1'000'000);
    return true;
}

void Renderer::RenderRectangleF(const RectangleF& rect, const glm::vec3 color) {
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    float vertices[] = {
        rect.x,     rect.y - rect.h, 0.0f, color.x, color.y, color.z,
        rect.x,     rect.y,          0.0f, color.x, color.y, color.z,
        rect.x + rect.w, rect.y - rect.h, 0.0f, color.x, color.y, color.z,
        rect.x,     rect.y,          0.0f, color.x, color.y, color.z,
        rect.x + rect.w, rect.y,     0.0f, color.x, color.y, color.z,
        rect.x + rect.w, rect.y - rect.h, 0.0f, color.x, color.y, color.z
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertices), std::end(vertices));
}

void Renderer::RenderRectangle(const Rectangle& rect, const glm::vec3 color) {
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

    const float colR = color.x;
    const float colG = color.y;
    const float colB = color.z;


    // pos.x, pos.y, pos.z,  col.r, col.g, col.b
    float vertices[] = {
        x,     y - h, 0.0f, color.x, color.y,color.z,
        x,     y,     0.0f,  color.x, color.y,color.z,
        x + w, y - h, 0.0f,color.x, color.y,color.z,
        x,     y,     0.0f,color.x, color.y,color.z,
        x + w, y,     0.0f,color.x, color.y,color.z,
        x + w, y - h, 0.0f, color.x, color.y,color.z
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertices), std::end(vertices));
}

void Renderer::RenderRectangleFEX(const RectangleF& rect, const glm::vec3 color, const float rotation) {
    // Włączenie atrybututów iwerzchołków
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    // Wierzchołki zdefiniowane względem środka prostokąta
    const float halfW = rect.w / 2.0f;
    const float halfH = rect.h / 2.0f;

    // Tworzymy macierz modelu (translacja + obrót)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(rect.x + halfW, rect.y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::vec4 transformed0 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed1 = model * glm::vec4(-halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed2 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed3 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed4 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed5 = model * glm::vec4(halfW, -halfH, 0.0f, 1.0f);
    const float vertex[] = {
        transformed0.x, transformed0.y , 0.0f , color.x , color.y,color.z,
        transformed1.x, transformed1.y , 0.0f , color.x , color.y,color.z,
        transformed2.x, transformed2.y , 0.0f , color.x , color.y,color.z,
        transformed3.x, transformed3.y , 0.0f , color.x , color.y,color.z,
        transformed4.x, transformed4.y , 0.0f , color.x , color.y,color.z,
        transformed5.x, transformed5.y , 0.0f , color.x , color.y,color.z
    };

    globalVertices.insert(globalVertices.end(), std::begin(vertex), std::end(vertex));
}


void Renderer::RenderRectangleEX(const Rectangle& rect, const glm::vec3 color, const float rotation) {
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

    // Tworzymy macierz modelu (translacja + obrót)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(temp.x + halfW, temp.y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));



    const glm::vec4 transformed0 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed1 = model * glm::vec4(-halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed2 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed3 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed4 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed5 = model * glm::vec4(halfW, -halfH, 0.0f, 1.0f);
    const float vertex[] = {
        transformed0.x, transformed0.y , 0.0f , color.x , color.y,color.z,
        transformed1.x, transformed1.y , 0.0f , color.x , color.y,color.z,
        transformed2.x, transformed2.y , 0.0f , color.x , color.y,color.z,
        transformed3.x, transformed3.y , 0.0f , color.x , color.y,color.z,
        transformed4.x, transformed4.y , 0.0f , color.x , color.y,color.z,
        transformed5.x, transformed5.y , 0.0f , color.x , color.y,color.z
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


    float halfW = rect.w / 2.0f;
    float halfH = rect.h / 2.0f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(rect.x + halfW, rect.y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));


    const glm::vec4 transformed0 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed1 = model * glm::vec4(-halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed2 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed3 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed4 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed5 = model * glm::vec4(halfW, -halfH, 0.0f, 1.0f);
    const float vertex[] = {
        transformed0.x, transformed0.y , 0.0f , 0.0f,0.0f,
        transformed1.x, transformed1.y , 0.0f , 0.0f,1.0f,
        transformed2.x, transformed2.y , 0.0f , 1.0f,1.0f,
        transformed3.x, transformed3.y , 0.0f , 0.0f,0.0f,
        transformed4.x, transformed4.y , 0.0f , 1.0f,1.0f,
        transformed5.x, transformed5.y , 0.0f , 1.0f,0.0f
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
    float aspect = static_cast<float>(H) / static_cast<float>(W);
    const float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    const float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    const float w = (static_cast<float>(rect.w) / W) * 2.0f;
    const float h = (static_cast<float>(rect.h) / H) * 2.0f * aspect;

    const float halfW = w / 2.0f;
    const float halfH = h / 2.0f;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x + halfW, y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::vec4 transformed0 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed1 = model * glm::vec4(-halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed2 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed3 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed4 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed5 = model * glm::vec4(halfW, -halfH, 0.0f, 1.0f);
    const float vertex[] = {
        transformed0.x, transformed0.y , 0.0f , 0.0f,0.0f,
        transformed1.x, transformed1.y , 0.0f , 0.0f,1.0f,
        transformed2.x, transformed2.y , 0.0f , 1.0f,1.0f,
        transformed3.x, transformed3.y , 0.0f , 0.0f,0.0f,
        transformed4.x, transformed4.y , 0.0f , 1.0f,1.0f,
        transformed5.x, transformed5.y , 0.0f , 1.0f,0.0f
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


    float halfW = rect.w / 2.0f;
    float halfH = rect.h / 2.0f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(rect.x + halfW, rect.y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    const float u0 = source.x;
    const float v0 = source.y;
    const float u1 = source.x + source.w;
    const float v1 = source.y + source.h;

    const glm::vec4 transformed0 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed1 = model * glm::vec4(-halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed2 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed3 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed4 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed5 = model * glm::vec4(halfW, -halfH, 0.0f, 1.0f);

    const float vertex[] = {
        transformed0.x, transformed0.y , 0.0f , u0, v0,
        transformed1.x, transformed1.y , 0.0f , u0, v1,
        transformed2.x, transformed2.y , 0.0f , u1, v1,

        transformed3.x, transformed3.y , 0.0f , u0, v0,
        transformed4.x, transformed4.y , 0.0f , u1, v1,
        transformed5.x, transformed5.y , 0.0f , u1, v0
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
    float aspect = static_cast<float>(H) / static_cast<float>(W);
    const float x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    const float y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    const float w = (static_cast<float>(rect.w) / W) * 2.0f;
    const float h = (static_cast<float>(rect.h) / H) * 2.0f *aspect;


    const float halfW = w / 2.0f;
    const float halfH = h / 2.0f;

    const float texW = static_cast<float>(texture.w);
    const float texH = static_cast<float>(texture.h);


    const float u0 = static_cast<float>(source.x) / texW;
    const float v0 = static_cast<float>(source.y) / texH;
    const float u1 = static_cast<float>(source.x + source.w) / texW;
    const float v1 = static_cast<float>(source.y + source.h) / texH;


    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x + halfW, y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));


    const glm::vec4 transformed0 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed1 = model * glm::vec4(-halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed2 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed3 = model * glm::vec4(-halfW, -halfH, 0.0f, 1.0f);
    const glm::vec4 transformed4 = model * glm::vec4(halfW, halfH, 0.0f, 1.0f);
    const glm::vec4 transformed5 = model * glm::vec4(halfW, -halfH, 0.0f, 1.0f);


    const float vertex[] = {
        transformed0.x, transformed0.y , 0.0f , u0, v0,
        transformed1.x, transformed1.y , 0.0f , u0, v1,
        transformed2.x, transformed2.y , 0.0f , u1, v1,

        transformed3.x, transformed3.y , 0.0f , u0, v0,
        transformed4.x, transformed4.y , 0.0f , u1, v1,
        transformed5.x, transformed5.y , 0.0f , u1, v0
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
}