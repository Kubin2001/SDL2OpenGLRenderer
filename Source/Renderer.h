#pragma once

#include "glm.hpp"
#include "ShaderLoader.h"

namespace MT {
	struct RectF {
		float x;
		float y;
		float w;
		float h;
	};

	struct Rect {
		int x;
		int y;
		int w;
		int h;
	};

	struct Color {
		unsigned char R, G, B;

		Color() : R(0), G(0), B(0) {}

		Color(const unsigned char R, const unsigned char G, const unsigned char B) : R(R), G(G), B(B) {}
	};

	struct ColorF {
		float R, G, B;

		ColorF() : R(0), G(0), B(0) {}

		ColorF(const float R, const float G, float B) : R(R), G(G), B(B) {}
	};

	class Texture {
		public:
		unsigned int w, h, texture;
		float alpha = 1.0f;

		void SetAlphaBending(const unsigned char A) {
			alpha = float(A) / 255;
		}
	};


	Texture LoadTexture(const char* path);
	


	class Renderer {

		private:
			 unsigned int currentProgram;
			 unsigned int textureLocation; //uniform
			 unsigned int renderCopyId;
			 unsigned int renderRectId;
			 unsigned int renderRectExId;
			 unsigned int renderCopyExId;
			 unsigned int RenderCopyExTransform;
			 unsigned int currentTexture;
			 unsigned int renderRectMatrixLoc;
			 unsigned int alphaLoc;

			 std::vector<float> globalVertices;



		public:
			 unsigned int VAO, VBO;
			 unsigned int W, H;


			 bool Start(unsigned int W, unsigned int H);

			 void ClearFrame(const unsigned char R, const unsigned char G, const unsigned char B);

			 void RenderRectF(const RectF& rect, const Color& col);
			 void RenderRect(const Rect& rect, const Color& col);

			 void RenderRectFEX(const RectF& rect, const Color& col, const float rotation);
			 void RenderRectEX(const Rect& rect, const Color& col, const float rotation);

			 void RenderCopyF(const RectF& rect, const Texture& texture);
			 void RenderCopy(const Rect& rect, const Texture& texture);

			 void RenderCopyPartF(const RectF& rect, const RectF& source, const Texture& texture);
			 void RenderCopyPart(const Rect& rect, const Rect& source, const Texture& texture);

			 void RenderCopyFEX(const RectF& rect, const Texture& texture, const float rotation);
			 void RenderCopyEX(const Rect& rect, const Texture& texture, const float rotation);

			 void RenderCopyPartFEX(const RectF& rect, const RectF& source, const Texture& texture, const float rotation);
			 void RenderCopyPartEX(const Rect& rect, const Rect& source, const Texture& texture, const float rotation);

			 void RenderPresent();

			 void Clear();
	};
}

