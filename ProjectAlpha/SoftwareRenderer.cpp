#include "SoftwareRenderer.h"

namespace ProjectAlpha
{
	namespace Renderer
	{
#define CONTEXT_CHECK ASSERT(!(_Context.IsCreated));

		//Let's go with context based rendering
		global_variable PAContext _Context;

		bool32 paCreateContext(GameMemory* memory, int32 width, int32 height, uint32 flags, PixelBuffer* pixelBuffer)
		{
			CONTEXT_CHECK;
			//allocate space for the context
			_Context.Viewport = { 0.0f, 0.0f, pixelBuffer->width, pixelBuffer->height };
			_Context.Modes = 0;
			_Context.FillMode = FILL;
			_Context.ClearColor = 0x00ffffff;
			_Context.Width = width;
			_Context.Height = height;
			_Context.DepthBuffer.width = width;
			_Context.DepthBuffer.height = height;
			_Context.DepthBuffer.pixels = (real32*)Memory::MemoryAlloc(memory->memory, sizeof(real32) * width * height);
			_Context.PixelBuffer = pixelBuffer;


			//Create render tiles
			int32 tcx = 4;
			int32 tcy = 4;
			int32 tw = width / tcx; //This could be x.0 to x.9 - ceil it
			int32 th = height / tcy;
			tw = ((tw + 3) / 4) * 4; //round up to nearest 4 for align
			th = ((th + 3) / 4) * 4; //round up to nearest 4 for align

			for (int32 tileY = 0; tileY < tcy; tileY++)
			{
				for (int32 tileX = 0; tileX < tcx; tileX++)
				{
					PARenderTile* tile = &_Context.Tiles[tileX + (tileY * tcx)];

					tile->MinX = tileX * tw;
					tile->MaxX = tile->MinX + tw;
					tile->MinY = tileY * th;
					tile->MaxY = tile->MinY + th;

					if (tileX == (tcx - 1))
					{
						tile->MaxX = width;
					}
					if (tileY == (tcy - 1))
					{
						tile->MaxY = height;
					}

					ASSERT((tile->MinX < width));
					ASSERT((tile->MinY < height));
				}
			}


			return((bool32)true);
		}

		void doAwesomeShit()
		{

			//
			//if(hasarea(fillrect))

			//		typedef int32 _m128i;
			//#define _mm_set1_epi8(x) x

			{
				int32 MaxX = 0;
				PARenderTile fillRect = {};

				__m128i ClipMask = _mm_set1_epi8(-1);

				__m128i StartupClipMask = _mm_set1_epi8(-1);
				__m128i StartClipMask = _mm_set1_epi8(-1);
				__m128i EndClipMask = _mm_set1_epi8(-1);

				__m128i StartClipMasks[] =
				{
					_mm_slli_si128(StartClipMask, 0 * 4),
					_mm_slli_si128(StartClipMask, 1 * 4),
					_mm_slli_si128(StartClipMask, 2 * 4),
					_mm_slli_si128(StartClipMask, 3 * 4)
				};

				__m128i EndClipMasks[] =
				{
					_mm_srli_si128(StartupClipMask, 0 * 4),
					_mm_srli_si128(StartupClipMask, 3 * 4),
					_mm_srli_si128(StartupClipMask, 2 * 4),
					_mm_srli_si128(StartupClipMask, 1 * 4)
				};

				if (fillRect.MinX & 3)
				{
					StartClipMask = StartClipMasks[fillRect.MinX & 3];
					fillRect.MinX = fillRect.MinX & ~3;
				}
				if (fillRect.MaxX & 3)
				{
					EndClipMask = EndClipMasks[fillRect.MaxX & 3];
					fillRect.MaxX = (fillRect.MaxX & ~3) + 4;
				}

				for (int32 XI = 0; XI < MaxX; XI += 4)
				{

					if ((XI + 8) < MaxX)
					{
						ClipMask = _mm_set1_epi8(-1);
					}
					else
					{
						ClipMask = EndClipMask;
					}
				}
			}

		}

		bool32 paDestroyContext(GameMemory* memory)
		{
			ASSERT((_Context.IsCreated));
			Memory::MemoryRelease(memory->memory, (void*)_Context.DepthBuffer.pixels);
			return((bool32)true);
		}

		void paSetProjectionMatrix(mat4& proj)
		{
			CONTEXT_CHECK;
			_Context.Projection = proj;
		}
		void paSetModelMatrix(mat4& model)
		{
			CONTEXT_CHECK;
			_Context.Model = model;
		}

		void paEnable(uint32 mode)
		{
			CONTEXT_CHECK;
			_Context.Modes |= mode;
		}

		void paDisable(uint32 mode)
		{
			CONTEXT_CHECK;
			_Context.Modes &= ~(1 << mode);
		}

		void paSetFillMode(uint32 fillMode)
		{
			CONTEXT_CHECK;
			_Context.FillMode = fillMode;
		}

		void paSetClearColor(uint32 clearColor)
		{
			CONTEXT_CHECK;
			_Context.ClearColor = clearColor;
		}

		void paClear(uint32 flags)
		{
			CONTEXT_CHECK;
			if (flags & COLOR_BIT)
			{
				uint32* pixel = (uint32*)_Context.PixelBuffer->memory;
				for (int32 i = 0; i < (_Context.PixelBuffer->width * _Context.PixelBuffer->height); i++)
					*(pixel++) = 0x00ffffff;
			}
			if (flags & DEPTH_BIT)
			{
				uint32* depthPixel = (uint32*)_Context.DepthBuffer.pixels;
				for (int32 i = 0; i < (_Context.DepthBuffer.width * _Context.DepthBuffer.height); i++)
					*(depthPixel++) = 0x0fff;
			}
		}

		void _paDrawPixel(int32 x, int32 y, uint32 color)
		{
			CONTEXT_CHECK;
			if (x < 0 || x >= _Context.PixelBuffer->width)
				return; //discard fragment
			if (y < 0 || y >= _Context.PixelBuffer->height)
				return; //discard fragment

			((uint32*)_Context.PixelBuffer->memory)[x + (y * _Context.PixelBuffer->width)] = color;
		}

		void _paFillRect(int32 x, int32 y, int32 w, int32 h, uint32 color)
		{
			CONTEXT_CHECK;
			for (int32 _x = x; _x < x + w; _x++)
			{
				if (_x < 0 || _x >= _Context.PixelBuffer->width)
					continue; //discard fragment
				for (int32 _y = y; _y < y + h; _y++)
				{
					if (_y < 0 || _y >= _Context.PixelBuffer->height)
						continue; //discard fragment
					//DrawPixel(pixelBuffer, _x, _y, color);
					((uint32*)_Context.PixelBuffer->memory)[_x + (_y * _Context.PixelBuffer->width)] = color;
				}
			}
		}

		PLATFORM_WORK_QUEUE_CALLBACK(paRasterizeScanLine)
		{
			_paRasterizeScanLine((PARenderTileOp*)data);
		}

		void _paRasterizeScanLine(PARenderTileOp* op)
		{

#define _MAX(x, y) (x > y ? x : y)
#define _MIN(x, y) (x > y ? y : x)
#define _CLAMP(x, minVal, maxVal) _MIN(_MAX(x, minVal), maxVal)
#define Interpolate(minVal, maxVal, gradient) (minVal + (maxVal - minVal) * _CLAMP(gradient, 0.0f, 1.0f))

			CONTEXT_CHECK;
			if (op->Line.y < 0 || op->Line.y >= _Context.PixelBuffer->height)
				return; //discard fragment

			//Calculate gradients

			real32 gradient1 = (op->Line.l1->v1->Position.y != op->Line.l1->v2->Position.y) ? (op->Line.y - op->Line.l1->v1->Position.y) / (op->Line.l1->v2->Position.y - op->Line.l1->v1->Position.y) : 1;
			real32 gradient2 = (op->Line.l2->v1->Position.y != op->Line.l2->v2->Position.y) ? (op->Line.y - op->Line.l2->v1->Position.y) / (op->Line.l2->v2->Position.y - op->Line.l2->v1->Position.y) : 1;

			//Start end X values
			int32 startX = (int32)Interpolate(op->Line.l1->v1->Position.x, op->Line.l1->v2->Position.x, gradient1);
			int32 endX = (int32)Interpolate(op->Line.l2->v1->Position.x, op->Line.l2->v2->Position.x, gradient2);

			//Start end Z values
			#define startZ ((real32)Interpolate(op->Line.l1->v1->Position.z, op->Line.l1->v2->Position.z, gradient1))
			#define endZ ((real32)Interpolate(op->Line.l2->v1->Position.z, op->Line.l2->v2->Position.z, gradient2))

			//left.GetDepth()
			//calc pixel gradient
			real32 gradient = startX / (real32)(endX - startX);
			real32 depth = Interpolate(startZ, endZ, gradient);

			if (_Context.FillMode & FILL)
			{
				int32 endX_c = _MIN(endX, _Context.PixelBuffer->width);
				uint32* pixel = ((uint32*)_Context.PixelBuffer->memory) + (op->Line.y * _Context.PixelBuffer->width) + (_MAX(startX, 0));
				for (int32 x = _MAX(startX, 0); x < endX_c; x++)
				{
					if (_Context.Modes & PA_DEPTH_TEST)
					{
						//check depth buffer
						if (depth < _Context.DepthBuffer.pixels[x + (op->Line.y * _Context.DepthBuffer.width)])
						{						
							//Depth write
							_Context.DepthBuffer.pixels[x + (op->Line.y * _Context.DepthBuffer.width)] = depth;
						}

						depth += op->Gradients.DepthXStep;
					}

					//Pixel write
					*(pixel++) = (uint32)(gradient1 * gradient2 * op->Line.color);
					
				}
			}
			else
			{
				if (!(startX < 0 || startX >= _Context.PixelBuffer->width))
					((uint32*)_Context.PixelBuffer->memory)[startX + (op->Line.y * _Context.PixelBuffer->width)] = op->Line.color;
				if (!(endX < 0 || endX >= _Context.PixelBuffer->width))
					((uint32*)_Context.PixelBuffer->memory)[endX + (op->Line.y * _Context.PixelBuffer->width)] = op->Line.color;
			}
		}

		inline void PerspectiveDivide(vec4& vec)
		{
			vec.x /= vec.w;
			vec.y /= vec.w;
			vec.z /= vec.w;
		}

		inline real32 CalcXStep(real32* values, real32 minYVert, real32 midYVert, real32 maxYVert, real32 oneOverDX)
		{
			return (((values[1] - values[2]) *
				(minYVert - maxYVert)) -
				((values[0] - values[2]) *
				(midYVert - maxYVert))) * oneOverDX;
		}

		inline real32 CalcYStep(real32* values, real32 minYVert, real32 midYVert, real32 maxYVert, real32 oneOverDY)
		{
			return (((values[1] - values[2]) *
				(minYVert - maxYVert)) -
				((values[0] - values[2]) *
				(midYVert - maxYVert))) * oneOverDY;
		}

		void paTriangle(Triangle& triangle, uint32 color)
		{
			CONTEXT_CHECK;

#if 1
			__paTriangle(triangle.v1, triangle.v2, triangle.v3, color);
			return;
#endif

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
			//We also need to look into C++ compiler and pass by reference overhead
			v1.Position = vec4(glm::project(vec3(v1.Position), _Context.Model, _Context.Projection, _Context.Viewport), 1.0f);
			v2.Position = vec4(glm::project(vec3(v2.Position), _Context.Model, _Context.Projection, _Context.Viewport), 1.0f);
			v3.Position = vec4(glm::project(vec3(v3.Position), _Context.Model, _Context.Projection, _Context.Viewport), 1.0f);
			PerspectiveDivide(v1.Position);
			PerspectiveDivide(v2.Position);
			PerspectiveDivide(v3.Position);

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

			//Inverse slopes, slope = dy/dx
			real32 topMiddleSlope, topBottomSlope;

			if (v2.Position.y - v1.Position.y >= 0)
			{
				topMiddleSlope = (v2.Position.x - v1.Position.x) / (v2.Position.y - v1.Position.y);
			}
			else
			{
				topMiddleSlope = 0;
			}

			if (v3.Position.y - v1.Position.y >= 0)
			{
				topBottomSlope = (v3.Position.x - v1.Position.x) / (v3.Position.y - v1.Position.y);
			}
			else
			{
				topBottomSlope = 0;
			}

			//Right side triangle
			//PARenderTileOp _op[16]; //This must change when we change tile count
			ScanLine line = {};
			Edge ETB = { &v1, &v3 };
			Edge ETM = { &v1, &v2 };
			Edge EMB = { &v2, &v3 };

#define CalcDX(v1, v2, v3) (  ((v2->Position.x - v3->Position.x) * (v1->Position.y - v3->Position.y)) - ( (v1->Position.x - v3->Position.x) * (v2->Position.y - v3->Position.y) ) )

			Vertex* ptrV1 = &v1;
			Vertex* ptrV2 = &v2;
			Vertex* ptrV3 = &v3;
			PARenderTileOp op;

			real32 _oneOverDX = 1.0f / CalcDX(ptrV1, ptrV2, ptrV3);
			real32 _oneOverDY = -_oneOverDX;

			op.Gradients.Depth[0] = v1.Position.z;
			op.Gradients.Depth[1] = v2.Position.z;
			op.Gradients.Depth[2] = v3.Position.z;
			op.Gradients.DepthXStep = CalcXStep(op.Gradients.Depth, v1.Position.y, v2.Position.y, v3.Position.y, _oneOverDX);
			op.Gradients.DepthYStep = CalcYStep(op.Gradients.Depth, v1.Position.x, v2.Position.x, v3.Position.x, _oneOverDX);

	

			for (int32 y = (int32)v1.Position.y; y < (int32)v3.Position.y; y++)
			{
				op.Tile = &_Context.Tiles[0];
				//right tri
				if (topMiddleSlope > topBottomSlope)
				{
					line.y = y;
					line.color = color;
					if (y < v2.Position.y)
					{
						line.l1 = &ETB;
						line.l2 = &ETM;
					}
					else
					{
						line.l1 = &ETB;
						line.l2 = &EMB;
					}
				}
				//Left tri
				else
				{
					line.y = y;
					line.color = color;
					if (y < v2.Position.y)
					{
						line.l1 = &ETM;
						line.l2 = &ETB;
					}
					else
					{
						line.l1 = &EMB;
						line.l2 = &ETB;
					}
				}
				op.Line = line;
				_paRasterizeScanLine(&op);
			}
		}

		inline real32 TriangleAreaTimesTwo(Vertex& a, Vertex& b, Vertex& c)
		{
			float x1 = b.Position.x - a.Position.x;
			float y1 = b.Position.y - a.Position.y;

			float x2 = c.Position.x - a.Position.x;
			float y2 = c.Position.y - a.Position.y;

			return (x1 * y2 - x2 * y1);
		}

		void __paScanEdges(Edge a, Edge b, bool32 handedness)
		{
			Edge left = a;
			Edge right = b;
			if (handedness)
			{
				Edge temp = left;
				left = right;
				right = temp;
			}

			int yStart = b.v1->Position.y;
			int yEnd = b.v2->Position.y;

			for (int32 y = yStart; y < yEnd; y++)
			{
				//Draw pixel
				_paDrawPixel(left.sX, y, 0x00ff0000);
				_paDrawPixel(right.sX, y, 0x000000ff);
				//left.step()
				//right.step()
			}
		}

		inline void CalculateEdge(Edge& e)
		{
			real32 yStart = ceil(e.v1->Position.y);
			real32 yEnd = ceil(e.v2->Position.y);
			real32 yDist = e.v2->Position.y - e.v1->Position.y;
			real32 xDist = e.v2->Position.x - e.v1->Position.x;
			real32 yPrestep = yStart - e.v1->Position.y;

			e.xStep = xDist / yDist;
			e.sX = e.v1->Position.x + yPrestep * e.xStep;

			real32 xPrestep = e.sX - e.v1->Position.x;
		}

		void __paTriangle(Vertex& _v1, Vertex& _v2, Vertex& _v3, uint32 color)
		{
			//Tri sort
			Vertex v1 = _v1;
			Vertex v2 = _v2;
			Vertex v3 = _v3;

			if (TriangleAreaTimesTwo(v1, v3, v2) >= 0)
			{
				return;
			}

			//TODO(meanzie): Optimize
			//We also need to look into C++ compiler and pass by reference overhead
			v1.Position = vec4(glm::project(vec3(v1.Position), _Context.Model, _Context.Projection, _Context.Viewport), 1.0f);
			v2.Position = vec4(glm::project(vec3(v2.Position), _Context.Model, _Context.Projection, _Context.Viewport), 1.0f);
			v3.Position = vec4(glm::project(vec3(v3.Position), _Context.Model, _Context.Projection, _Context.Viewport), 1.0f);
			PerspectiveDivide(v1.Position);
			PerspectiveDivide(v2.Position);
			PerspectiveDivide(v3.Position);

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

			Edge topToBottom = { &v1, &v3 };
			Edge topToMiddle = { &v1, &v2 };
			Edge middleToBottom = { &v2, &v3 };

			CalculateEdge(topToBottom);
			CalculateEdge(topToMiddle);
			CalculateEdge(middleToBottom);

			bool32 handedness = TriangleAreaTimesTwo(v1, v3, v2) >= 0 ? true : false;

			__paScanEdges(topToBottom, topToMiddle, handedness);
			__paScanEdges(topToBottom, middleToBottom, handedness);
		}
	}
}