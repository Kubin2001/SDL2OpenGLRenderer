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

struct MethaneColor {
	unsigned char R, G, B;

	MethaneColor() : R(0), G(0), B(0) {}

	MethaneColor(const unsigned char R, const unsigned char G, const unsigned char B): R(R),G(G),B(B){}
};

struct MethaneColorF {
	float R, G, B;

	MethaneColorF() : R(0), G(0), B(0) {}

	MethaneColorF(const float R, const float G, float B) : R(R), G(G), B(B) {}
};

class MethaneTexture {
	public:
		unsigned int w, h, texture;
		float alpha = 1.0f;

		void SetAlphaBending(const unsigned char A) {
			alpha = float(A) / 255;
		}
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
		static unsigned int renderRectMatrixLoc;
		static unsigned int alphaLoc;

		static std::vector<float> globalVertices;



	public:
		static unsigned int VAO, VBO, EBO;
		static glm::mat4 tarnsMatrix;
		static unsigned int W, H;
		static unsigned int indecies[6];


		static bool Start(unsigned int W , unsigned int H);

		static void ClearFrame(const unsigned char R, const unsigned char G, const unsigned char B);

		static void RenderRectF(const RectangleF & rect, const MethaneColor& col);
		static void RenderRect(const Rectangle& rect, const MethaneColor& col);

		static void RenderRectFEX(const RectangleF& rect, const MethaneColor &col, const float rotation);
		static void RenderRectEX(const Rectangle& rect, const MethaneColor& col, const float rotation);

		static void RenderCopyF(const RectangleF& rect, const MethaneTexture& texture);
		static void RenderCopy(const Rectangle& rect, const MethaneTexture& texture);

		static void RenderCopyPartF(const RectangleF& rect, const RectangleF& source, const MethaneTexture& texture);
		static void RenderCopyPart(const Rectangle& rect, const Rectangle& source, const MethaneTexture& texture);

		static void RenderCopyFEX(const RectangleF& rect, const MethaneTexture& texture, const float rotation);
		static void RenderCopyEX(const Rectangle& rect, const MethaneTexture& texture, const float rotation);

		static void RenderCopyPartFEX(const RectangleF& rect, const RectangleF& source, const MethaneTexture& texture, const float rotation);
		static void RenderCopyPartEX(const Rectangle& rect, const Rectangle& source, const MethaneTexture& texture, const float rotation);

		static void RenderPresent();

		static void Clear();
};