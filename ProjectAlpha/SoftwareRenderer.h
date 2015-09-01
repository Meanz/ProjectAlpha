#pragma once

#include "ProjectAlpha.h"
#include "ctdfs.h"

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
			real32* pixels;
			int32 width;
			int32 height;
		};

		struct RenderBuffer
		{
			PixelBuffer pixelBuffer;
			DepthBuffer16 depthBuffer;
			int32 width;
			int32 height;
		};

		//A render tile
		struct PARenderTile
		{
			int32 MinX;
			int32 MaxX;
			int32 MinY;
			int32 MaxY;
		};

		//This should be aligned to 32 unless pointers aren't 32, which would make things awkward
		struct PAContext
		{
			PixelBuffer* PixelBuffer;
			DepthBuffer16 DepthBuffer;
			int32 Width;
			int32 Height;
			bool32 IsCreated;

			//Render stuff
			PARenderTile Tiles[4*4];

			//4*32bit floats?
			vec4 Viewport;
			mat4 Projection;
			mat4 Model;

			uint32 Modes;

			uint32 FillMode;
			uint32 ClearColor;
		};

		extern "C"
		{

			struct Gradients
			{
				real32 Depth[3];
				real32 DepthXStep;
				real32 DepthYStep;


			};

			struct Edge
			{
				Vertex* v1;
				Vertex* v2;

				real32 xStep;
				real32 sX;

			};

			__declspec(align(16)) struct ScanLine
			{
				int32 y;
				Edge* l1;
				Edge* l2;
				uint32 color;
			};

			struct PARenderTileOp
			{
				PARenderTile* Tile;
				ScanLine Line;
				Gradients Gradients;
			};
		}

		enum
		{
			PA_DEPTH_TEST = 0x1,
		};

		enum
		{
			FILL = 0x1,
			LINES = 0x2,
		};

		enum
		{
			COLOR_BIT = 0x1,
			DEPTH_BIT = 0x2,
		};

		bool32 paCreateContext(GameMemory* memory, int32 width, int32 height, uint32 flags, PixelBuffer* pixelBuffer);
		bool32 paDestroyContext();

		void paSetProjectionMatrix(mat4& proj);
		void paSetModelMatrix(mat4& model);

		//enable,disable
		void paEnable(uint32 mode);
		void paDisable(uint32 mode);

		//Clear
		void paSetFillMode(uint32 fillMode);
		void paSetClearColor(uint32 clearColor);
		void paClear(uint32 clearFlags);

		void _paDrawPixel(int32 x, int32 y, uint32 color);
		void _paFillRect(int32 x, int32 y, int32 w, int32 h, uint32 color);
		PLATFORM_WORK_QUEUE_CALLBACK(paRasterizeScanLine);
		//void _paRasterizeScanLine(int32 y, Vertex& v1, Vertex& v2, Vertex& v3, Vertex& v4, uint32 color);
		void _paRasterizeScanLine(PARenderTileOp* op);
		void paTriangle(Triangle& triangle, uint32 color);

		void __paScanEdges(Edge a, Edge b, bool32 handedness);
		void __paTriangle(Vertex& v1, Vertex& v2, Vertex& v3, uint32 color);

		
	}
}