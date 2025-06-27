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

unsigned int Renderer::indecies[6] = { 0, 1, 2, //Indeksy 1 trójk¹ta
                                      0, 2, 3 }; // indeksy 2 trójk¹ta


bool Renderer::Start(unsigned int W, unsigned int H) {
    Renderer::W = W;
    Renderer::H = H;
    // Deklaracja zmiennych dla Vertex Array Object (VAO) i Vertex Buffer Object (VBO)
    // Generowanie VAO (Vertex Array Object) - obiekt przechowuj¹cy konfiguracjê atrybutów wierzcho³ków
    glGenVertexArrays(1, &VAO);

    // Generowanie VBO (Vertex Buffer Object) - bufor przechowuj¹cy dane wierzcho³ków
    glGenBuffers(1, &VBO);

    //Generowanie ebo aby nie musieæ u¿ywaæ 6 wie¿cho³ków
    glGenBuffers(1, &EBO);

    // Bindowanie VAO - od tego momentu wszystkie operacje na VAO bêd¹ dotyczyæ tego obiektu
    glBindVertexArray(VAO);

    // Bindowanie VBO - od tego momentu wszystkie operacje na VBO bêd¹ dotyczyæ tego bufora
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //Ustawnianie ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecies), indecies, GL_STATIC_DRAW);

    // Konfiguracja atrybutu wierzcho³ka - mówi OpenGL, jak interpretowaæ dane w buforze
    // 0 - indeks atrybutu (w shaderze odpowiada location = 0)
    // 3 - liczba sk³adowych (x, y, z)
    // GL_FLOAT - typ danych
    // GL_FALSE - czy normalizowaæ dane (nie w tym przypadku)
    // 3 * sizeof(float) - odleg³oœæ miêdzy kolejnymi wierzcho³kami (w bajtach)
    // (void*)0 - przesuniêcie do pierwszego elementu w buforze

    //Gdy nie ma tekstur
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // powierzchnie
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float))); // kolory

    //Gdy s¹ tekstury
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // powierzchnie
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // tekstury


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);




    // Przyœpieszacze
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

void Renderer::RenderRectangle(Rectangle& rect, glm::vec3 color) {
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
    // -1.0f dlatego ¿e nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego ¿e w innym wypadku ekran by³by traktowany jakby mia³ powójn¹ szerokoœæ
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
    // W³¹czenie atrybututów iwerzcho³ków
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    // Wierzcho³ki zdefiniowane wzglêdem œrodka prostok¹ta
    const float halfW = rect.w / 2.0f;
    const float halfH = rect.h / 2.0f;

    // Tworzymy macierz modelu (translacja + obrót)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(rect.x + halfW, rect.y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // Wierzcho³ki wzglêdem œrodka
    glm::vec3 localVertices[6] = {
        {-halfW, -halfH, 0.0f},
        {-halfW,  halfH, 0.0f},
        { halfW,  halfH, 0.0f},

        {-halfW, -halfH, 0.0f},
        { halfW,  halfH, 0.0f},
        { halfW, -halfH, 0.0f}
    };

    for (int i = 0; i < 6; i++) {
        glm::vec4 transformed = model * glm::vec4(localVertices[i], 1.0f);
        globalVertices.emplace_back(transformed.x);
        globalVertices.emplace_back(transformed.y);
        globalVertices.emplace_back(transformed.z);
        globalVertices.emplace_back(color.r);
        globalVertices.emplace_back(color.g);
        globalVertices.emplace_back(color.b);
    }
}


void Renderer::RenderRectangleEX(Rectangle& rect, glm::vec3 color, float rotation) {
    // W³¹czenie atrybututów iwerzcho³ków
    if (Renderer::currentProgram != Renderer::renderRectId) {
        RenderPresent();
        Renderer::currentProgram = Renderer::renderRectId;
        glUseProgram(Renderer::renderRectId);
    }

    RectangleF temp;
    // -1.0f dlatego ¿e nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego ¿e w innym wypadku ekran by³by traktowany jakby mia³ powójn¹ szerokoœæ
    temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    temp.y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    temp.h = (static_cast<float>(rect.h) / H) * 2.0f;

    // Wersja gdzie punkt 0.0 jest w lewym dolnym
    // -1.0f dlatego ¿e nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego ¿e w innym wypadku ekran by³by traktowany jakby mia³ powójn¹ szerokoœæ
    //temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    //temp.y = (static_cast<float>(rect.y) / H) * 2.0f - 1.0f;
    //temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    //temp.h = (static_cast<float>(rect.h) / H) * 2.0f;
    //std::cout <<temp.x<<"  " << temp.y << "\n";

    // Wierzcho³ki zdefiniowane wzglêdem œrodka prostok¹ta
    // Wierzcho³ki zdefiniowane wzglêdem œrodka prostok¹ta
    float halfW = temp.w / 2.0f;
    float halfH = temp.h / 2.0f;

    // Tworzymy macierz modelu (translacja + obrót)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(temp.x + halfW, temp.y - halfH, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // Wierzcho³ki wzglêdem œrodka
    glm::vec3 localVertices[6] = {
        {-halfW, -halfH, 0.0f},
        {-halfW,  halfH, 0.0f},
        { halfW,  halfH, 0.0f},

        {-halfW, -halfH, 0.0f},
        { halfW,  halfH, 0.0f},
        { halfW, -halfH, 0.0f}
    };

    for (int i = 0; i < 6; i++) {
        glm::vec4 transformed = model * glm::vec4(localVertices[i], 1.0f);
        globalVertices.emplace_back(transformed.x);
        globalVertices.emplace_back(transformed.y);
        globalVertices.emplace_back(transformed.z);
        globalVertices.emplace_back(color.r);
        globalVertices.emplace_back(color.g);
        globalVertices.emplace_back(color.b);
    }
}



void Renderer::RenderCopyF(RectangleF& rect, const MethaneTexture& texture) {
    // W³¹czenie atrybututów iwerzcho³ków
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
    glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, ¿e przypisujemy teksturê do GL_TEXTURE0


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

void Renderer::RenderCopy(Rectangle& rect, const MethaneTexture& texture) {
    float x = (rect.x / static_cast<float>(W)) * 2.0f - 1.0f;
    float y = 1.0f - (rect.y / static_cast<float>(H)) * 2.0f;
    float w = (rect.w / static_cast<float>(W)) * 2.0f;
    float h = (rect.h / static_cast<float>(H)) * 2.0f;

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

    // Przypisanie tekstury do samplera (uniforma 'texture1') - tekstura 0
    glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, ¿e przypisujemy teksturê do GL_TEXTURE0

    //    // pos.x, pos.y, pos.z,  col.r, col.g, col.b,  tex.u, tex.v
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

void Renderer::RenderCopyPartF(RectangleF& rect, RectangleF& source, const MethaneTexture& texture) {
    // aktywacja tekstury
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.texture);

    // W³¹czenie atrybututów iwerzcho³ków
    if (Renderer::currentProgram != Renderer::renderCopyId) {
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }


    // Przypisanie tekstury do samplera (uniforma 'texture1') - tekstura 0
    glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, ¿e przypisujemy teksturê do GL_TEXTURE0

    float u0 = source.x;
    float v0 = source.y;
    float u1 = source.x + source.w;
    float v1 = source.y + source.h;

    float vertices[] = {
        // pos.x,          pos.y,           pos.z,   tex.u, tex.v
        rect.x,          rect.y - rect.h,  0.0f,    u0, v0,
        rect.x,          rect.y,           0.0f,    u0, v1,
        rect.x + rect.w, rect.y,           0.0f,    u1, v1,
        rect.x + rect.w, rect.y - rect.h,  0.0f,    u1, v0
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}

void Renderer::RenderCopyPart(Rectangle& rect, Rectangle& source, const MethaneTexture &tex) {
    RectangleF temp;
    temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    temp.y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    temp.h = (static_cast<float>(rect.h) / H) * 2.0f;


    // aktywacja tekstury
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.texture);

    // W³¹czenie atrybututów iwerzcho³ków
    if (Renderer::currentProgram != Renderer::renderCopyId) {
        Renderer::currentProgram = Renderer::renderCopyId;
        glUseProgram(Renderer::renderCopyId);
    }
    // Przypisanie tekstury do samplera (uniforma 'texture1') - tekstura 0
    glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, ¿e przypisujemy teksturê do GL_TEXTURE0

    RectangleF tempSource;

    tempSource.x = static_cast<float>(source.x) / tex.w;
    tempSource.y = static_cast<float>(source.y) / tex.h;
    tempSource.w = static_cast<float>(source.w) / tex.w;
    tempSource.h = static_cast<float>(source.h) / tex.h;

    float u0 = tempSource.x;
    float v0 = tempSource.y;
    float u1 = tempSource.x + tempSource.w;
    float v1 = tempSource.y + tempSource.h;

    float vertices[] = {
        // pos.x,          pos.y,           pos.z,   tex.u, tex.v
        temp.x,          temp.y - temp.h,  0.0f,    u0, v0,
        temp.x,          temp.y,           0.0f,    u0, v1,
        temp.x + temp.w, temp.y,           0.0f,    u1, v1,
        temp.x + temp.w, temp.y - temp.h,  0.0f,    u1, v0
    };


    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::RenderCopyFEX(RectangleF& rect, const MethaneTexture& texture, float rotation) {
    if (Renderer::currentProgram != Renderer::renderCopyExId) {
        Renderer::currentProgram = Renderer::renderCopyExId;
        glUseProgram(Renderer::renderCopyExId);
    }

    // aktywacja tekstury
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.texture);


    // Przypisanie tekstury do samplera (uniforma 'texture1') - tekstura 0
    glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, ¿e przypisujemy teksturê do GL_TEXTURE0


    float halfW = rect.w / 2.0f;
    float halfH = rect.h / 2.0f;

    float vertices[] = {
        // pos.x, pos.y, pos.z,   tex.u, tex.v
       -halfW, -halfH,   0.0f,     0.0f, 0.0f,
       -halfW,  halfH,   0.0f,     0.0f, 1.0f,
        halfW,  halfH,   0.0f,     1.0f, 1.0f,
        halfW, -halfH,   0.0f,     1.0f, 0.0f
    };



    // Tworzymy macierz modelu
    glm::mat4 model = glm::mat4(1.0f);

    // Najpierw translacja do pozycji prostok¹ta (œrodek)
    model = glm::translate(model, glm::vec3(rect.x + halfW, rect.y - halfH, 0.0f));

    // Potem obrót wokó³ Z
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    float aspect = static_cast<float>(W) / static_cast<float>(H);
    model = glm::scale(model, glm::vec3(1.0f, aspect, 1.0f));

    // Przes³anie macierzy do shadera
    glUniformMatrix4fv(Renderer::RenderCopyExTransform, 1, GL_FALSE, glm::value_ptr(model));

    // Za³aduj dane do bufora
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Rysuj ca³y prostok¹t (2 trójk¹ty -> 6 wierzcho³ków)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // W³¹czenie atrybututów iwerzcho³ków
}

void Renderer::RenderCopyEX(Rectangle& rect, const MethaneTexture& texture, float rotation) {
    if (Renderer::currentProgram != Renderer::renderCopyExId) {
        Renderer::currentProgram = Renderer::renderCopyExId;
        glUseProgram(Renderer::renderCopyExId);
    }

    // aktywacja tekstury
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.texture);


    // Przypisanie tekstury do samplera (uniforma 'texture1') - tekstura 0
    glUniform1i(Renderer::textureLocation, 0); // 0 oznacza, ¿e przypisujemy teksturê do GL_TEXTURE0
    RectangleF temp;
    // -1.0f dlatego ¿e nieznormalizowana jest od -1.0 a nie 0.0
    // * 2.0f dlatego ¿e w innym wypadku ekran by³by traktowany jakby mia³ powójn¹ szerokoœæ
    temp.x = (static_cast<float>(rect.x) / W) * 2.0f - 1.0f;
    temp.y = 1.0f - (static_cast<float>(rect.y) / H) * 2.0f;
    temp.w = (static_cast<float>(rect.w) / W) * 2.0f;
    temp.h = (static_cast<float>(rect.h) / H) * 2.0f;


    float halfW = temp.w / 2.0f;
    float halfH = temp.h / 2.0f;

    float vertices[] = {
        // pos.x, pos.y, pos.z,   tex.u, tex.v
       -halfW, -halfH,   0.0f,     0.0f, 0.0f,
       -halfW,  halfH,   0.0f,     0.0f, 1.0f,
        halfW,  halfH,   0.0f,     1.0f, 1.0f,
        halfW, -halfH,   0.0f,     1.0f, 0.0f
    };



    // Tworzymy macierz modelu
    glm::mat4 model = glm::mat4(1.0f);

    // Najpierw translacja do pozycji prostok¹ta (œrodek)
    model = glm::translate(model, glm::vec3(temp.x + halfW, temp.y - halfH, 0.0f));

    // Potem obrót wokó³ Z
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

    // Przes³anie macierzy do shadera
    glUniformMatrix4fv(Renderer::RenderCopyExTransform, 1, GL_FALSE, glm::value_ptr(model));

    // Za³aduj dane do bufora
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Rysuj ca³y prostok¹t (2 trójk¹ty -> 6 wierzcho³ków)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // W³¹czenie atrybututów iwerzcho³ków
}


void Renderer::Clear() {
    // Odwi¹zanie VAO - bezpieczne praktyka, aby nie modyfikowaæ przypadkowo tego VAO w przysz³oœci
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // czyszczenie aby nie by³o wycieków pamiêci nie tworzyæ jak vbo i vao s¹ globalnie zadeklarowane
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
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