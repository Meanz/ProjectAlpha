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

		struct VertexBuffer
		{
			Vertex* vertices;
			uint32 numVertices;
		};

		struct IndexBuffer
		{
			uint32* indices;
			uint32 numIndices;
		};

		//An edge is basically a line
		struct Edge
		{
			real32 yStart;
			real32 yEnd;
			real32 xStep;
			real32 gradient;
		};

		inline void DrawPixel(PixelBuffer pixelBuffer, int32 x, int32 y, uint32 color);
		inline void FillRect(PixelBuffer pixelBuffer, int32 x, int32 y, int32 w, int32 h, uint32 color);

		real32 Interpolate(real32 min, real32 max, real32 gradient);

		void RasterizeScanLine(PixelBuffer pixelBuffer, int32 y, Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4, uint32 color);

		inline void PerspectiveDivide(vec4& v)
		{
			v.x /= v.w;
			v.y /= v.w;
			v.z /= v.w;
		}

		inline void InitScreenSpace(mat4& mat, real32 halfWidth, real32 halfHeight)
		{
			//((vec4*)mat.values)[0] = vec4{ halfWidth, 0, 0, halfWidth };
			//((vec4*)mat.values)[1] = vec4{ 0, -halfHeight, 0, halfHeight };
			//((vec4*)mat.values)[2] = vec4{ 0, 0, 1, 0 };
			//((vec4*)mat.values)[3] = vec4{ 0, 0, 0, 1 };
		}

		void RenderTriangle(PixelBuffer pixelBuffer, mat4& proj, mat4& model, Triangle& triangle);
	}
}