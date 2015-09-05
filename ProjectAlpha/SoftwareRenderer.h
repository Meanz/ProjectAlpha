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
			vec2 UV;
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

		//A render tile
		struct PARenderTile
		{
			int32 MinX;
			int32 MaxX;
			int32 MinY;
			int32 MaxY;
		};

		struct PATexture
		{
			void* Pixels;
			int32 Width;
			int32 Height;
			int32 BytesPerPixel;
		};

		struct PAContext
		{
			PixelBuffer* PixelBuffer;
			DepthBuffer16 DepthBuffer;
			int32 Width;
			int32 Height;
			bool32 IsCreated;

			//Render stuff
			PARenderTile Tiles[4*4];
			PATexture* BoundTexture;

			vec4 Viewport;
			mat4 Projection;
			mat4 Model;
			mat4 View;

			uint32 Modes;

			uint32 FillMode;
			uint32 ClearColor;

			vec4 DrawColor;

			vec3 DirectionalLight;
		};

		extern "C"
		{

			struct Gradients
			{
				real32 Depth[3];
				real32 OneOverZ[3];
				real32 TexCoordX[3];
				real32 TexCoordY[3];
				real32 LightAmt[3];

				real32 DepthXStep;
				real32 DepthYStep;
				real32 TexCoordXXStep;
				real32 TexCoordXYStep;
				real32 TexCoordYXStep;
				real32 TexCoordYYStep;
				real32 LightAmtXStep;
				real32 LightAmtYStep;
				real32 OneOverZXStep;
				real32 OneOverZYStep;
			};

			struct Edge
			{
				Vertex* v1;
				Vertex* v2;

				real32 xStep;
				real32 x;

				real32 texCoordX;
				real32 texCoordXStep;

				real32 texCoordY;
				real32 texCoordYStep;

				real32 depth;
				real32 depthStep;

				real32 LightAmt;
				real32 LightAmtStep;

				real32 OneOverZ;
				real32 OneOverZStep;

			};
		}

		enum
		{
			PA_DEPTH_TEST = 0x1,
			PA_TEXTURE = 0x2,
		};

		enum
		{
			PA_FILL = 0x1,
			PA_LINES = 0x2,
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
		void paSetViewMatrix(mat4& view);

		//enable,disable
		void paEnable(uint32 mode);
		void paDisable(uint32 mode);

		//
		void paSetDirectonalLight(vec3 lightDir);

		//
		void paBindTexture(PATexture* pTexture);
		
		//
		void paSetDrawColor(vec4 color);

		//Clear
		void paSetFillMode(uint32 fillMode);
		void paSetClearColor(uint32 clearColor);
		void paClear(uint32 clearFlags);

		void _paDrawPixel(int32 x, int32 y, uint32 color);
		void _paFillRect(int32 x, int32 y, int32 w, int32 h, uint32 color);
		void _paScanEdges(Gradients& g, Edge& a, Edge& b, bool32 handedness);

		void paTriangle(Triangle& triangle);
		void paTriangle(Vertex& v1, Vertex& v2, Vertex& v3);
		void paRasterizeTriangle(Vertex& v1, Vertex& v2, Vertex& v3);

		
		

		
	}
}