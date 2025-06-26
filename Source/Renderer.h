#pragma once

#include "glm.hpp"
#include "ShaderLoader.h"
struct RectangleF {
	float x;
	float y;
	float w;
	float h;
};

struct Rectangle {
	int x;
	int y;
	int w;
	int h;
};

struct MethaneTexture {
	unsigned int w, h, texture;
};

class Renderer {

	private:
		static unsigned int currentProgram;
		static unsigned int textureLocation; //uniform
		static unsigned int renderCopyId;
		static unsigned int renderRectId;
		static unsigned int renderRectExId;
		static unsigned int renderCopyExId;
		static unsigned int RenderCopyExTransform;
		static unsigned int currentTexture;

		static std::vector<float> globalVertices;


	public:
		static unsigned int VAO, VBO, EBO;
		static glm::mat4 tarnsMatrix;
		static unsigned int W, H;
		static unsigned int indecies[6];


		static bool Start(unsigned int W , unsigned int H);

		static void RenderRectangle(Rectangle& rect, glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f));

		static void RenderRectangleF(const RectangleF & rect, const glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f));

		static void RenderRectangleEX(Rectangle& rect, glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f), float rotation = 0.0f);

		static void RenderRectangleFEX(const RectangleF& rect, const glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f), const float rotation = 0.0f);

		static void RenderCopyF(RectangleF& rect, const MethaneTexture& texture);
		static void RenderCopy(Rectangle& rect, const MethaneTexture& texture);

		static void RenderCopyPartF(RectangleF& rect, RectangleF& source, const MethaneTexture& texture);

		static void RenderCopyPart(Rectangle& rect, Rectangle& source, const MethaneTexture& texture);

		static void RenderCopyFEX(RectangleF& rect, const MethaneTexture& texture, float rotation = 0.0f);
		static void RenderCopyEX(Rectangle& rect, const MethaneTexture& texture, float rotation = 0.0f);

		static void RenderPresent();

		static void Clear();

};