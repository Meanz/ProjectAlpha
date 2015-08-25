#pragma once

#include "ProjectAlpha.h"
#include "ctdfs.h"
#include <glm/gtc/matrix_transform.hpp>

namespace ProjectAlpha
{
	namespace Renderer
	{

		struct Vertex
		{
			vec4 Position;
			vec3 Normal;
			vec3 UV;
		};

		struct Triangle
		{
			Vertex v1;
			Vertex v2;
			Vertex v3;
		};

		struct DepthBuffer16
		{
			int16* pixels;
			uint32 width;
			uint32 height;
		};

		struct Renderbuffer
		{
			PixelBuffer* PixelBuffer;
			DepthBuffer16* DepthBuffer;
		};

		void DrawPixel(PixelBuffer pixelBuffer, int32 x, int32 y, uint32 color);
		void FillRect(PixelBuffer pixelBuffer, int32 x, int32 y, int32 w, int32 h, uint32 color);
		real32 Interpolate(real32 min, real32 max, real32 gradient);
		void RasterizeScanLine(PixelBuffer pixelBuffer, int32 y, Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4, uint32 color);
		void RenderTriangle(PixelBuffer pixelBuffer, mat4& proj, mat4& model, Triangle& triangle, uint32 color);
	}
}