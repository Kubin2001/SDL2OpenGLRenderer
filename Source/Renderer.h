#pragma once

#include "glm.hpp"
#include "ShaderLoader.h"
#include <SDL.h>

namespace MT {

	SDL_GLContext Innit(SDL_Window* window);

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


	Texture* LoadTexture(const char* path);

	Texture* LoadTextureFromSurface(SDL_Surface *surf);
	

	class Renderer {

		private:
			SDL_Window* window = nullptr;
			 SDL_GLContext context;
			 unsigned int currentProgram;
			 
			 unsigned int renderCopyId;
			 unsigned int renderRectId;
			 unsigned int renderCopyCircleId;
			 unsigned int renderCircleId;
			 unsigned int renderRectAlphaId;
			 unsigned int renderCopyFilterId;

			 unsigned int RenderCopyExTransform;
			 unsigned int textureLocation;
			 unsigned int currentTexture;
			 unsigned int renderRectMatrixLoc;
			 unsigned int alphaLoc;
			 unsigned int alphaLocRect;
			 unsigned int radiusLoc;
			 unsigned int radiusLoc2;
			 unsigned int alphaLocFilter;
			 unsigned int textureLocationFilter;
			 float currentRadius;

			 std::vector<float> globalVertices;



		public:
			 unsigned int VAO, VBO;
			 int W, H;


			 bool Start(SDL_Window* window, SDL_GLContext context);

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

			 void RenderCopyCircle(const Rect& rect, const Texture& texture, const float radius = 0.5f);

			 void RenderCircle(const Rect& rect, const Color& col, const float radius = 0.5f);

			 void RenderRectAlpha(const Rect& rect, const Color& col, unsigned char alpha);
			 void RenderRectAlphaEX(const Rect& rect, const Color& col, unsigned char alpha, const float rotation);

			 void RenderCopyFiltered(const Rect& rect, const Texture& texture, const Color& filter);

			 void RenderCopyPartFiltered(const Rect& rect, const Rect& source, const Texture& texture, const Color& filter);

			 void RenderPresent(bool switchContext = true);

			 void Clear();
	};
}

