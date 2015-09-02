#include "SoftwareRenderer.h"

namespace ProjectAlpha
{
	namespace Renderer
	{

#define _MAX(x, y) (x > y ? x : y)
#define _MIN(x, y) (x > y ? y : x)
#define _CLAMP(x, minVal, maxVal) _MIN(_MAX(x, minVal), maxVal)
#define Interpolate(minVal, maxVal, gradient) (minVal + (maxVal - minVal) * _CLAMP(gradient, 0.0f, 1.0f))


#define CONTEXT_CHECK ASSERT(!(_Context.IsCreated));

		//Let's go with context based rendering
		global_variable PAContext _Context;

		bool32 paCreateContext(GameMemory* memory, int32 width, int32 height, uint32 flags, PixelBuffer* pixelBuffer)
		{
			CONTEXT_CHECK;
			//allocate space for the context
			_Context.Viewport = { 0.0f, 0.0f, (real32)pixelBuffer->width, (real32)pixelBuffer->height };
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
		void paSetViewMatrix(mat4& view)
		{
			CONTEXT_CHECK;
			_Context.View = view;
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

		inline bool32 IsInsideViewFrustum(vec4 m_pos)
		{
			return (bool32)(abs(m_pos.x) <= abs(m_pos.w) &&
				abs(m_pos.y) <= abs(m_pos.w) &&
				abs(m_pos.z) <= abs(m_pos.w));
		}

		void paTriangle(Triangle& triangle, uint32 color)
		{
			CONTEXT_CHECK;

			//Perform vertex transformation
			triangle.v1.Position = _Context.Projection * _Context.View * triangle.v1.Position;
			triangle.v2.Position = _Context.Projection * _Context.View * triangle.v2.Position;
			triangle.v3.Position = _Context.Projection * _Context.View * triangle.v3.Position;

			if (!IsInsideViewFrustum(triangle.v1.Position) && !IsInsideViewFrustum(triangle.v2.Position) && !IsInsideViewFrustum(triangle.v3.Position))
			{
				__paTriangle(triangle.v1, triangle.v2, triangle.v3, color);
			}
		}

		inline real32 TriangleAreaTimesTwo(Vertex& a, Vertex& b, Vertex& c)
		{
			real32 x1 = b.Position.x - a.Position.x;
			real32 y1 = b.Position.y - a.Position.y;

			real32 x2 = c.Position.x - a.Position.x;
			real32 y2 = c.Position.y - a.Position.y;

			return (x1 * y2 - x2 * y1);
		}

		void __paScanEdges(Edge& a, Edge& b, bool32 handedness)
		{
			Edge& left = handedness ? b : a;
			Edge& right = handedness ? a : b;
			int32 yStart = (int32)_MAX(ceil(b.v1->Position.y), 0);
			int32 yEnd = (int32)_MIN(ceil(b.v2->Position.y), _Context.PixelBuffer->height);

			//Draw point
#if 0
			int32 pointSize = 3;
			int32 xMin = (int32)_MAX(ceil(left.sX), 0);
			int32 xMax = (int32)_MIN(ceil(right.sX), _Context.PixelBuffer->width);

			for (int32 x = xMin - pointSize; x < xMin + pointSize; x++){
				for (int32 y = yStart - pointSize; y < yStart + pointSize; y++)
				{

					if (x < 0 || x >= _Context.Width || y < 0 || y >= _Context.Height)
						continue;
					*(((uint32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + x)) = 0x000000ff;
				}
			}

			xMax = (int32)_MIN(ceil(right.sX + (right.xStep * (yEnd - yStart))), _Context.PixelBuffer->width);
			for (int32 x = xMax - pointSize; x < xMax + pointSize; x++){
				for (int32 y = yEnd - pointSize; y < yEnd + pointSize; y++)
				{
					if (x < 0 || x >= _Context.Width || y < 0 || y >= _Context.Height)
						continue;
					*(((uint32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + x)) = 0x0055ff55;
				}
			}
#endif
			for (int32 y = yStart; y < yEnd; y++)
			{
				//Draw pixel
				int32 xMin = (int32)_MAX(ceil(left.sX), 0);
				int32 xMax = (int32)_MIN(ceil(right.sX), _Context.PixelBuffer->width);

				uint32* pixel = ((uint32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + xMin);
				for (int32 x = xMin; x < xMax; x++)
				{
					if (*pixel != _Context.ClearColor)
					{
						pixel++;
						continue;
					}
					*(pixel++) = (0x00ff0000);
				}
				//if (xMin <= xMax)
				{
					//*(((uint32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + xMin)) = 0x000000ff;
					//*(((uint32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + xMax)) = 0x00ff0000;
				}

				//Left.Step(), Right.Step()
				left.sX += left.xStep;
				right.sX += right.xStep;
			}
		}

		void CalculateEdge(Edge& e)
		{
			real32 yStart = ceil(e.v1->Position.y);
			real32 yEnd = ceil(e.v2->Position.y);
			real32 yDist = e.v2->Position.y - e.v1->Position.y;
			real32 xDist = e.v2->Position.x - e.v1->Position.x;
			real32 yPrestep = yStart - e.v1->Position.y;

			e.xStep = xDist / yDist;
			e.sX = e.v1->Position.x + (yPrestep * e.xStep);

			real32 xPrestep = e.sX - e.v1->Position.x;
		}

		void PerspectiveDivide(v4* vec)
		{
			//If w is 0 then we have a problem :P
			if (vec->w == 0) return;
			vec->x /= vec->w;
			vec->y /= vec->w;
			vec->z /= vec->w;
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

			mat4 screenSpaceTransform;
			InitScreenSpace(screenSpaceTransform, _Context.Width / 2, _Context.Height / 2);

			//ScreenSpaceTransform
			v1.Position = screenSpaceTransform * v1.Position;
			v2.Position = screenSpaceTransform * v2.Position;
			v3.Position = screenSpaceTransform * v3.Position;

			//Perspective divide
			PerspectiveDivide(&v1.Position);
			PerspectiveDivide(&v2.Position);
			PerspectiveDivide(&v3.Position);

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