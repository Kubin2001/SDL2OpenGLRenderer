#include "ShaderLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

int ShaderLoader::success;
char ShaderLoader::infoLog[512];
std::unordered_map<std::string, unsigned int> ShaderLoader::shaders;
std::unordered_map<std::string, unsigned int> ShaderLoader::shaderPrograms;

std::string LoadShaderFile(const char* fileName) {
    std::ifstream file;
    std::stringstream buf;

    std::string ret = "";
    file.open(fileName);
    if (file.is_open()) {
        buf << file.rdbuf();
        ret = buf.str();

        std::cout << "Shader: " << fileName << " loaded...\n";
    }
    else
    {
        std::cout << "Error file: " << fileName << " not openned\n";
    }
    file.close();

    return ret;

}

void ShaderLoader::LoadShader(const std::string& name, const std::string& path, GLenum shaderType) {
    //Compile Fragment Shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(shaderType);
    std::string fragShaderSrc = LoadShaderFile(path.c_str());
    const GLchar* fragShader = fragShaderSrc.c_str();
    glShaderSource(fragmentShader, 1, &fragShader, nullptr);

    glCompileShader(fragmentShader);

    //catch compile error
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);

        std::cout << "Error with vertex shader compilation: \n" << infoLog << "\n";
    }
    else{
        shaders[name] = fragmentShader;
        std::cout << "Compilation succesfull\n";
    }
}

unsigned int& ShaderLoader::GetShader(const std::string& name) {
    if (shaders.find(name) != shaders.end()) {
        return shaders[name];
    }
    else{
        std::cout << "Shader not found\n";
    }

}


void ShaderLoader::CreateShaderProgram(std::vector<std::string> &names, const std::string& programName) {
    for (auto& it : names) {
        if (shaders.find(it) == shaders.end()) {
            std::cout << "Shader " << it << "not found program creation stopped\n";
            return;
        }
    }
    

    unsigned int shaderProgram = 0;

    shaderProgram = glCreateProgram();
    shaderPrograms[programName] = shaderProgram;
    for (auto& it : names) {
        glAttachShader(shaderPrograms[programName], ShaderLoader::GetShader(it));
    }
    glLinkProgram(shaderPrograms[programName]);

    //catch error
    glGetProgramiv(shaderPrograms[programName], GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderPrograms[programName], 512, nullptr, infoLog);
        std::cout << "Shader Linking error\n" << "Error: " << infoLog << "\n";

        std::cout << "Error with vertex shader compilation: \n" << infoLog << "\n";
    }
    else
    {
        std::cout << "Linking succesfull\n";
    }

    //Usuwanie shader�w bo z racji po��czenia w program s� niepotrzebne
    for (auto& it : names) {
        glDeleteShader(ShaderLoader::GetShader(it));
        shaders.erase(it);
    }
}

unsigned int& ShaderLoader::GetProgram(const std::string& name) {
    if (shaderPrograms.find(name) != shaderPrograms.end()) {
        return shaderPrograms[name];
    }
    else {
        std::cout << "Shader not found\n";
    }
}