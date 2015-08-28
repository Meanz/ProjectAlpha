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

		struct RenderBuffer
		{
			PixelBuffer pixelBuffer;
			DepthBuffer16 depthBuffer;
			int32 width;
			int32 height;
		};

		struct PAContext
		{
			PixelBuffer* pixelBuffer;
			DepthBuffer16 depthBuffer;
			int32 width;
			int32 height;
			bool32 IsCreated;

			mat4 proj;
			mat4 model;
		};

		bool32 paCreateContext(GameMemory* memory, int32 width, int32 height, uint32 flags, PixelBuffer* pixelBuffer);
		bool32 paDestroyContext();

		void paSetProjectionMatrix(mat4& proj);
		void paSetModelMatrix(mat4& model);

		real32 Interpolate(real32 min, real32 max, real32 gradient);
		void _paFillRect(int32 x, int32 y, int32 w, int32 h, uint32 color);
		void _paRasterizeScanLine(int32 y, Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4, uint32 color);
		void paTriangle(Triangle& triangle, uint32 color);

		
	}
}