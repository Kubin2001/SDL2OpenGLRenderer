#pragma once

#include <unordered_map>
#include <string>
#include "glad\glad.h"
#include <vector>


class ShaderLoader {
	private:
		static int success;
		static char infoLog[512];
		static std::unordered_map<std::string,unsigned int> shaders;
		static std::unordered_map<std::string, unsigned int> shaderPrograms;

	public:
		static void LoadShader(const std::string& name, const std::string& path, GLenum shaderType);

		static unsigned int& GetShader(const std::string& name);

		static void CreateShaderProgram(std::vector<std::string> &names, const std::string& programName);

		static unsigned int& GetProgram(const std::string& name);


};
