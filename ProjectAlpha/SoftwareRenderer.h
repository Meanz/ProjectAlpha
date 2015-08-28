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
			PixelBuffer* PixelBuffer;
			DepthBuffer16 DepthBuffer;
			int32 Width;
			int32 Height;
			bool32 IsCreated;

			mat4 Projection;
			mat4 Model;

			uint32 ClearColor;
		};

		enum
		{
			COLOR_BIT = 0x1,
			DEPTH_BIT = 0x2
		};

		bool32 paCreateContext(GameMemory* memory, int32 width, int32 height, uint32 flags, PixelBuffer* pixelBuffer);
		bool32 paDestroyContext();

		void paSetProjectionMatrix(mat4& proj);
		void paSetModelMatrix(mat4& model);

		void paSetClearColor(uint32 clearColor);
		void paClear(uint32 clearFlags);

		real32 Interpolate(real32 min, real32 max, real32 gradient);
		void _paFillRect(int32 x, int32 y, int32 w, int32 h, uint32 color);
		void _paRasterizeScanLine(int32 y, Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4, uint32 color);
		void paTriangle(Triangle& triangle, uint32 color);

		
	}
}