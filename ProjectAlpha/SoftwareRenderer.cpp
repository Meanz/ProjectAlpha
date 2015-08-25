#include "SoftwareRenderer.h"

namespace ProjectAlpha
{
	namespace Renderer
	{
		inline void DrawPixel(PixelBuffer pixelBuffer, int32 x, int32 y, uint32 color)
		{
			//Pixel bounds check
			int32 pixelPos = x + (y * pixelBuffer.width);
			//ASSERT(!(pixelPos < 0 || (uint32)pixelPos >= (pixelBuffer.size / 4)));
			if (pixelPos < 0 || (uint32)pixelPos >= (pixelBuffer.size / 4)) return;
			((uint32*)pixelBuffer.memory)[pixelPos] = color;
		}

		inline void FillRect(PixelBuffer pixelBuffer, int32 x, int32 y, int32 w, int32 h, uint32 color)
		{
			for (int32 _x = x; _x < x + w; _x++)
			{
				for (int32 _y = y; _y < y + h; _y++)
				{
					DrawPixel(pixelBuffer, _x, _y, color);
				}
			}
		}

		real32 Interpolate(real32 min, real32 max, real32 gradient)
		{
			return min + (max - min) * glm::clamp(gradient, 0.0f, 1.0f);
		}

		void RasterizeScanLine(PixelBuffer pixelBuffer, int32 y, Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4, uint32 color)
		{
			real32 gradient1 = (v1.Position.y != v2.Position.y) ? (y - v1.Position.y) / (v2.Position.y - v1.Position.y) : 1;
			real32 gradient2 = (v3.Position.y != v4.Position.y) ? (y - v3.Position.y) / (v4.Position.y - v3.Position.y) : 1;

			int32 startX = (int32)Interpolate(v1.Position.x, v2.Position.x, gradient1);
			int32 endX = (int32)Interpolate(v3.Position.x, v4.Position.x, gradient2);

			for (int32 x = startX; x < endX; x++)
			{
				if (x < 0 || x > pixelBuffer.width)
					continue; //discard fragment
				DrawPixel(pixelBuffer, x, y, color);
			}
		}

		void RenderTriangle(PixelBuffer pixelBuffer, mat4& proj, mat4& world, Triangle& triangle)
		{
			Vertex v1 = triangle.v1;
			Vertex v2 = triangle.v2;
			Vertex v3 = triangle.v3;

			/*
			mat4 screenSpaceTransform;
			InitScreenSpace(screenSpaceTransform, pixelBuffer.width / (real32)2, pixelBuffer.height / (real32)2);

			//ScreenSpaceTransform
			v1.Position = screenSpaceTransform * v1.Position;
			v2.Position = screenSpaceTransform * v2.Position;
			v3.Position = screenSpaceTransform * v3.Position;

			//Perspective divide
			PerspectiveDivide(v1.Position);
			PerspectiveDivide(v2.Position);
			PerspectiveDivide(v3.Position);
			*/

			//TODO(meanzie): Optimize
			vec4 viewport = { 0.0f, 0.0f, pixelBuffer.width, pixelBuffer.height };
			v1.Position = vec4(glm::project(vec3(v1.Position), world, proj, viewport), 1.0f);
			v2.Position = vec4(glm::project(vec3(v2.Position), world, proj, viewport), 1.0f);
			v3.Position = vec4(glm::project(vec3(v3.Position), world, proj, viewport), 1.0f);

			//glm::unProject(vec3(1.0f), model, proj, viewport)


			//Sort the triangles by Y values least to most

			//Is no3 smaller than no2 ?
			if (v3.Position.y < v2.Position.y) {
				Vertex temp = v3;
				v3 = v2;
				v2 = temp;
			}

			//Is no2 smaller than no1 ?
			if (v2.Position.y < v1.Position.y) {
				Vertex temp = v2;
				v2 = v1;
				v1 = temp;
			}

			//Are we sure that nothing changed between no3 and no2?
			if (v3.Position.y < v2.Position.y) {
				Vertex temp = v3;
				v3 = v2;
				v2 = temp;
			}

			//Inverse slopes
			real32 dV1V2, dV1V3;

			if (v2.Position.y - v1.Position.y > 0)
			{
				dV1V2 = (v2.Position.x - v1.Position.x) / (v2.Position.y - v1.Position.y);
			}
			else
			{
				dV1V2 = 0;
			}

			if (v3.Position.y - v1.Position.y > 0)
			{
				dV1V3 = (v3.Position.x - v1.Position.x) / (v3.Position.y - v1.Position.y);
			}
			else
			{
				dV1V3 = 0;
			}

			//Right side triangle
			if (dV1V2 > dV1V3)
			{
				for (int32 y = (int32)v1.Position.y; y < (int32)v3.Position.y; y++)
				{
					if (y < v2.Position.y)
					{
						RasterizeScanLine(pixelBuffer, y, v1, v3, v1, v2, 0x00ff0000);
					}
					else
					{
						RasterizeScanLine(pixelBuffer, y, v1, v3, v2, v3, 0x00ff0000);
					}
				}
			}
			//Left side triangle
			else
			{
				for (int32 y = (int32)v1.Position.y; y < (int32)v3.Position.y; y++)
				{
					if (y < v2.Position.y)
					{
						RasterizeScanLine(pixelBuffer, y, v1, v2, v1, v3, 0x00ff0000);
					}
					else
					{
						RasterizeScanLine(pixelBuffer, y, v2, v3, v1, v3, 0x00ff0000);
					}
				}
			}
		}
	}
}