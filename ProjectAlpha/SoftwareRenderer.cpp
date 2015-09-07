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


		inline bool32 IsInsideViewFrustum(vec4 m_pos)
		{
			return (bool32)(abs(m_pos.x) <= abs(m_pos.w) &&
				abs(m_pos.y) <= abs(m_pos.w) &&
				abs(m_pos.z) <= abs(m_pos.w));
		}

		inline real32 TriangleAreaTimesTwo(Vertex& a, Vertex& b, Vertex& c)
		{
			real32 x1 = b.Position.x - a.Position.x;
			real32 y1 = b.Position.y - a.Position.y;

			real32 x2 = c.Position.x - a.Position.x;
			real32 y2 = c.Position.y - a.Position.y;

			return (x1 * y2 - x2 * y1);
		}

		inline void PerspectiveDivide(v4* vec)
		{
			//If w is 0 then we have a problem :P
			if (vec->w == 0) return;
			vec->x /= vec->w;
			vec->y /= vec->w;
			vec->z /= vec->w;
		}



		//Let's go with context based rendering
		global_variable PAContext _Context;

		bool32 paCreateContext(GameMemory* memory, int32 width, int32 height, uint32 flags, PixelBuffer* pixelBuffer)
		{
			CONTEXT_CHECK;
			//allocate space for the context
			_Context.Viewport = { 0.0f, 0.0f, (real32)pixelBuffer->width, (real32)pixelBuffer->height };
			_Context.Modes = 0;
			_Context.FillMode = PA_FILL;
			_Context.ClearColor = 0x00ffffff;
			_Context.DrawColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
			_Context.Width = width;
			_Context.Height = height;
			_Context.DepthBuffer.width = width;
			_Context.DepthBuffer.height = height;
			_Context.DepthBuffer.pixels = (real32*)Memory::MemoryAlloc(memory->memory, sizeof(real32) * width * height);
			_Context.PixelBuffer = pixelBuffer;
			_Context.BoundTexture = nullptr;
			_Context.DirectionalLight = vec3(0.2f, -0.6f, 0.2f);


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

		void paSetDirectonalLight(vec3 lightDir)
		{
			CONTEXT_CHECK;
			_Context.DirectionalLight = lightDir;
		}

		void paBindTexture(PATexture* pTexture)
		{
			CONTEXT_CHECK;
			_Context.BoundTexture = pTexture;
		}

		void paSetDrawColor(vec4 color)
		{
			CONTEXT_CHECK;
			_Context.DrawColor = color;
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
				return;
			if (y < 0 || y >= _Context.PixelBuffer->height)
				return;

			((uint32*)_Context.PixelBuffer->memory)[x + (y * _Context.PixelBuffer->width)] = color;
		}

		void _paFillRect(int32 x, int32 y, int32 w, int32 h, uint32 color)
		{
			CONTEXT_CHECK;
			for (int32 _x = x; _x < x + w; _x++)
			{
				if (_x < 0 || _x >= _Context.PixelBuffer->width)
					continue;
				for (int32 _y = y; _y < y + h; _y++)
				{
					if (_y < 0 || _y >= _Context.PixelBuffer->height)
						continue;
					//DrawPixel(pixelBuffer, _x, _y, color);
					((uint32*)_Context.PixelBuffer->memory)[_x + (_y * _Context.PixelBuffer->width)] = color;
				}
			}
		}

		void Step(Edge& e)
		{
			e.x += e.xStep;
			e.texCoordX += e.texCoordXStep;
			e.texCoordY += e.texCoordYStep;
			e.LightAmt += e.LightAmtStep;
			e.OneOverZ += e.OneOverZStep;
		}

#include <Windows.h>
		global_variable r64 pcFreq;
		global_variable u64 cntStart;

		void TimingStart()
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			pcFreq = double(li.QuadPart) / 1000000.0;
			QueryPerformanceCounter(&li);
			cntStart = li.QuadPart;
		}

		double TimingEnd()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return double(li.QuadPart - cntStart) / pcFreq;
		}

		void paSetCycles(r64 cycles)
		{
			CONTEXT_CHECK;
			_Context._cycles = cycles;
		}

		r64 paGetCycles()
		{
			CONTEXT_CHECK;
			return _Context._cycles;
		}

		void paAddCycles(r64 cycles)
		{
			_Context._cycles += cycles;
			_Context._cyclesCount++;
		}

		u32 paGetCyclesCount()
		{
			CONTEXT_CHECK;
			return _Context._cyclesCount;
		}
		void paResetCycles()
		{
			CONTEXT_CHECK;
			_Context._cycles = 0;
			_Context._cyclesCount = 0;
		}



		void _paScanEdges(Gradients& g, Edge& a, Edge& b, bool32 handedness)
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

			TimingStart();
			for (int32 y = yStart; y < yEnd; y++)
			{

#if 0
				int32 xMin = (int32)_MAX(ceil(left.x), 0);
				int32 xMax = (int32)_MIN(ceil(right.x), _Context.PixelBuffer->width);
				real32 xPrestep = xMin - left.x;
				real32 texCoordX = left.texCoordX + g.TexCoordXXStep * xPrestep;
				real32 texCoordY = left.texCoordY + g.TexCoordYXStep * xPrestep;
				real32 oneOverZ = left.OneOverZ + g.OneOverZXStep * xPrestep;
				real32 depth = left.depth * g.DepthXStep * xPrestep;
				real32 lightAmt = left.LightAmt + g.LightAmtXStep * xPrestep;

				u32* pixel = ((u32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + xMin);
				for (int32 x = xMin; x < xMax; x++)
				{

					r32 z = 1.0f / oneOverZ;
					i32 srcX = (i32)((texCoordX * z) * (r32)(255 - 1) + 0.5f);
					i32 srcY = (i32)((texCoordY * z) * (r32)(255 - 1) + 0.5f);

					v4 color = _Context.DrawColor;
					if (_Context.Modes & PA_TEXTURE)
					{
						//Do stuff
					}

					//do lighting calculation
					color.x = color.x * lightAmt;
					color.y = color.y * lightAmt;
					color.z = color.z * lightAmt;
					*(pixel) = (((u32)(color.w * 255) << 24) | ((u32)(color.x * 255) << 16) | ((u32)(color.y * 255) << 8) | ((u32)(color.z * 255) << 0));

					//Increment locals
					//XStep
					oneOverZ += g.OneOverZXStep;
					texCoordX += g.TexCoordXXStep;
					texCoordY += g.TexCoordYXStep;
					depth += g.DepthXStep;
					lightAmt += g.LightAmtXStep;

					//Advance pixels
					pixel ++;
				}
#else

#define _f128(x, y) ((r32*)&x)[y]
				int32 xMin = (int32)_MAX(ceil(left.x), 0);
				int32 xMax = (int32)_MIN(ceil(right.x), _Context.PixelBuffer->width);
				//real32 depth = left.depth * g.DepthXStep * xPrestep;

				__m128 texWidth_4x = _mm_set1_ps(254.0f); //width - 1 
				__m128 texHeight_4x = _mm_set1_ps(254.0f); //width - 1
				__m128 _255_4x = _mm_set1_ps(255.0f);
				__m128 half_4x = _mm_set1_ps(0.5f);
				__m128 one_4x = _mm_set1_ps(1.0f);
				
				r32 xPrestep = xMin - left.x;
				__m128 _xPrestep_4x = _mm_set1_ps(xPrestep);

#define CalcInitialCoord(l, r) _mm_add_ps(l, _mm_mul_ps(r, _xPrestep_4x))

				__m128 texCoordX_4x = _mm_set1_ps(left.texCoordX);
				__m128 texCoordXXStep_4x = _mm_set1_ps(g.TexCoordXXStep);
				__m128 initialTexCoordX_4x = CalcInitialCoord(texCoordX_4x, texCoordXXStep_4x);

				__m128 texCoordY_4x = _mm_set1_ps(left.texCoordY);
				__m128 texCoordYXStep_4x = _mm_set1_ps(g.TexCoordYXStep);
				__m128 initialTexCoordY_4x = CalcInitialCoord(texCoordY_4x, texCoordYXStep_4x);

				__m128 oneOverZ_4x = _mm_set1_ps(left.OneOverZ);
				__m128 oneOverZXStep_4x = _mm_set1_ps(g.OneOverZXStep);
				__m128 initialOneOverZ_4x = CalcInitialCoord(oneOverZ_4x, oneOverZXStep_4x);

				__m128 lightAmt_4x = _mm_set1_ps(left.LightAmt);
				__m128 lightAmtXStep_4x = _mm_set1_ps(g.LightAmtXStep);
				__m128 initialLightAmt_4x = CalcInitialCoord(lightAmt_4x, lightAmtXStep_4x);

				u32* pixel = ((u32*)_Context.PixelBuffer->memory) + ((y * _Context.PixelBuffer->width) + xMin);
				for (int32 x = xMin; x < xMax; x += 4)
				{
					//All x variables here
					__m128 texCoordX, texCoordY, oneOverZ, lightAmt;
					__m128 colorR, colorG, colorB, colorA;
					__m128 srcX, srcY;
					__m128 z;


					i32 xOff = x - xMin;
					__m128 _mmxOff = _mm_set_ps((r32)xOff + 0, (r32)xOff + 1, (r32)xOff + 2, (r32)xOff + 3);

					//texCoordX = _mm_add_ps(texCoordX, _mm_mul_ps(_mm_set1_ps(g.TexCoordXXStep), _xPrestep_4x));
					//texCoordX = _mm_add_ps(texCoordX, _mm_mul_ps(_mm_set1_ps(g.TexCoordXXStep), _mmxOff));
					
					//Light amt
					//_f128(lightAmt, i) = (left.LightAmt) + (g.LightAmtXStep * xPrestep) + (g.LightAmtXStep * (i + xOff));
#define CalcCoord(amt, xstep) _mm_add_ps(amt, _mm_add_ps(_mm_mul_ps(xstep, _xPrestep_4x), _mm_mul_ps(xstep, _mmxOff)))
#define CalcCoord2(l, r) _mm_add_ps(l, _mm_mul_ps(r, _mmxOff))

					texCoordX = CalcCoord2(initialTexCoordX_4x, texCoordXXStep_4x);
					texCoordY = CalcCoord2(initialTexCoordY_4x, texCoordYXStep_4x);
					oneOverZ = CalcCoord2(initialOneOverZ_4x, oneOverZXStep_4x);
					lightAmt = CalcCoord2(initialLightAmt_4x, lightAmtXStep_4x);

					z = _mm_div_ps(one_4x, oneOverZ);

					for (int i = 0; i < 4; i++)
					{
						//Color!
						_f128(colorR, i) = _Context.DrawColor.x;
						_f128(colorG, i) = _Context.DrawColor.y;
						_f128(colorB, i) = _Context.DrawColor.z;
						_f128(colorA, i) = _Context.DrawColor.w;
					}

					//i32 srcX = (i32)((texCoordX * z) * (r32)(255 - 1) + 0.5f);
					srcX = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(texCoordX, z), texWidth_4x), half_4x);
					srcY = _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(texCoordY, z), texHeight_4x), half_4x);

					//Lighting multiplication
					colorR = _mm_mul_ps(colorR, lightAmt);
					colorG = _mm_mul_ps(colorG, lightAmt);
					colorB = _mm_mul_ps(colorB, lightAmt);

					//Unpacks

					//Convert from 0-1 linear space to 0-255 sRGB space
					colorR = _mm_mul_ps(_255_4x, colorR);

					//Convert floats to ints
#if 1
					__m128i IntR = _mm_cvtps_epi32(colorR);
					__m128i IntG = _mm_cvtps_epi32(colorG);
					__m128i IntB = _mm_cvtps_epi32(colorB);
					__m128i IntA = _mm_cvtps_epi32(colorA);


					__m128i out = _mm_or_si128(_mm_or_si128(_mm_or_si128(
						_mm_slli_epi32(IntR, 16), 
						_mm_slli_epi32(IntG, 8)), 
						IntB), 
						_mm_slli_epi32(IntA, 24));

					*((__m128i*)pixel) = out;
#else
					//  
					//			|B3|B2|B1|B0
					// unpacklo
					//			|R3|R2|R1|R0
					//	=>
					//			|R1|B1|R0|B0

					__m128i R1B1R0B0 = _mm_unpacklo_epi32(_mm_castps_si128(colorB), _mm_castps_si128(colorR));
					__m128i A1G1A0G0 = _mm_unpacklo_epi32(_mm_castps_si128(colorG), _mm_castps_si128(colorA));

					__m128i R3B3R2B2 = _mm_unpackhi_epi32(_mm_castps_si128(colorB), _mm_castps_si128(colorR));
					__m128i A3G3A2G2 = _mm_unpackhi_epi32(_mm_castps_si128(colorG), _mm_castps_si128(colorA));

					__m128i ARGB0 = _mm_unpacklo_epi32(R1B1R0B0, A1G1A0G0);
					__m128i ARGB1 = _mm_unpackhi_epi32(R1B1R0B0, A1G1A0G0);

					__m128i ARGB2 = _mm_unpacklo_epi32(R3B3R2B2, A3G3A2G2);
					__m128i ARGB3 = _mm_unpackhi_epi32(R3B3R2B2, A3G3A2G2);

					//Write
					for (int i = 0; i < 4; i++)
					{
						*(pixel + i) = (
							((u32)(_f128(colorA, i) * 255) << 24) |
							((u32)(_f128(colorR, i) * 255) << 16) |
							((u32)(_f128(colorG, i) * 255) << 8) |
							((u32)(_f128(colorB, i) * 255) << 0));
					}
#endif

					
					pixel += 4;
				}



#endif

				//Left.Step(), Right.Step()
				//YStep
				Step(left);
				Step(right);
			}
			paAddCycles(TimingEnd());

		}

		void paTriangle(Triangle& triangle)
		{
			CONTEXT_CHECK;

			//Perform vertex transformation
			triangle.v1.Position = _Context.Projection * _Context.View * triangle.v1.Position;
			triangle.v2.Position = _Context.Projection * _Context.View * triangle.v2.Position;
			triangle.v3.Position = _Context.Projection * _Context.View * triangle.v3.Position;

			if (!IsInsideViewFrustum(triangle.v1.Position) && !IsInsideViewFrustum(triangle.v2.Position) && !IsInsideViewFrustum(triangle.v3.Position))
			{
				paRasterizeTriangle(triangle.v1, triangle.v2, triangle.v3);
			};
		}

		void paTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
		{
			CONTEXT_CHECK;

			//Perform vertex transformation
			v1.Position = _Context.Projection * _Context.View * v1.Position;
			v2.Position = _Context.Projection * _Context.View * v2.Position;
			v3.Position = _Context.Projection * _Context.View * v3.Position;

			if (!IsInsideViewFrustum(v1.Position) && !IsInsideViewFrustum(v2.Position) && !IsInsideViewFrustum(v3.Position))
			{
				paRasterizeTriangle(v1, v2, v3);
			}
		}

		void CalculateEdge(Edge& e, Gradients& g, int32 i)
		{
			real32 yStart = ceil(e.v1->Position.y);
			real32 yEnd = ceil(e.v2->Position.y);
			real32 yDist = e.v2->Position.y - e.v1->Position.y;
			real32 xDist = e.v2->Position.x - e.v1->Position.x;
			real32 yPrestep = yStart - e.v1->Position.y;

			e.xStep = xDist / yDist;
			e.x = e.v1->Position.x + (yPrestep * e.xStep);

			real32 xPrestep = e.x - e.v1->Position.x;

			e.texCoordX = g.TexCoordX[i] + g.TexCoordXXStep * xPrestep + g.TexCoordXYStep * yPrestep;
			e.texCoordXStep = g.TexCoordXYStep + g.TexCoordXXStep * e.xStep;

			e.texCoordY = g.TexCoordY[i] + g.TexCoordYXStep * xPrestep + g.TexCoordYYStep * yPrestep;
			e.texCoordYStep = g.TexCoordYYStep + g.TexCoordYXStep * e.xStep;

			e.LightAmt = g.LightAmt[i] + g.LightAmtXStep * xPrestep + g.LightAmtYStep * yPrestep;
			e.LightAmtStep = g.LightAmtYStep + g.LightAmtXStep * e.xStep;

			e.OneOverZ = g.OneOverZ[i] + g.OneOverZXStep * xPrestep + g.OneOverZYStep * yPrestep;
			e.OneOverZStep = g.OneOverZYStep + g.OneOverZXStep * e.xStep;
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

		inline real32 Saturate(real32 val)
		{
			if (val > 1.0f)
				return 1.0f;
			if (val < 0.0f)
				return 0.0f;
			return val;
		}

		void CalculateGradients(Gradients& g, Vertex& v1, Vertex& v2, Vertex& v3)
		{

			real32 oneOverdX = 1.0f / (((v2.Position.x - v3.Position.x) * (v1.Position.y - v3.Position.y)) -
				((v1.Position.x - v3.Position.x) * (v2.Position.y - v3.Position.y)));

			real32 oneOverdY = -oneOverdX;


			g.Depth[0] = v1.Position.z;
			g.Depth[1] = v2.Position.z;
			g.Depth[2] = v3.Position.z;

			vec3 lightDir = _Context.DirectionalLight;
			g.LightAmt[0] = Saturate(Dot(v1.Normal, lightDir)) * 0.9f + 0.1f;
			g.LightAmt[1] = Saturate(Dot(v2.Normal, lightDir)) * 0.9f + 0.1f;
			g.LightAmt[2] = Saturate(Dot(v3.Normal, lightDir)) * 0.9f + 0.1f;

			//Note that the W component is the perspective Z value
			//The Z component is the occlusion Z value
			g.OneOverZ[0] = 1.0f / v1.Position.w;
			g.OneOverZ[1] = 1.0f / v2.Position.w;
			g.OneOverZ[2] = 1.0f / v3.Position.w;

			g.TexCoordX[0] = v1.UV.x * g.OneOverZ[0];
			g.TexCoordX[1] = v2.UV.x * g.OneOverZ[1];
			g.TexCoordX[2] = v3.UV.x * g.OneOverZ[2];

			g.TexCoordY[0] = v1.UV.y * g.OneOverZ[0];
			g.TexCoordY[1] = v2.UV.y * g.OneOverZ[1];
			g.TexCoordY[2] = v3.UV.y * g.OneOverZ[2];

			real32 v1y = v1.Position.y;
			real32 v2y = v2.Position.y;
			real32 v3y = v3.Position.y;

			real32 v1x = v1.Position.x;
			real32 v2x = v2.Position.x;
			real32 v3x = v3.Position.x;

			g.TexCoordXXStep = CalcXStep(g.TexCoordX, v1y, v2y, v3y, oneOverdX);
			g.TexCoordXYStep = CalcYStep(g.TexCoordX, v1x, v2x, v3x, oneOverdY);
			g.TexCoordYXStep = CalcXStep(g.TexCoordY, v1y, v2y, v3y, oneOverdX);
			g.TexCoordYYStep = CalcYStep(g.TexCoordY, v1x, v2x, v3x, oneOverdY);

			g.DepthXStep = CalcXStep(g.Depth, v1y, v2y, v3y, oneOverdX);
			g.DepthYStep = CalcYStep(g.Depth, v1x, v2x, v3x, oneOverdY);

			g.LightAmtXStep = CalcXStep(g.LightAmt, v1y, v2y, v3y, oneOverdX);
			g.LightAmtYStep = CalcYStep(g.LightAmt, v1x, v2x, v3x, oneOverdY);

			g.OneOverZXStep = CalcXStep(g.OneOverZ, v1y, v2y, v3y, oneOverdX);
			g.OneOverZYStep = CalcYStep(g.OneOverZ, v1x, v2x, v3x, oneOverdY);
		}

		void paRasterizeTriangle(Vertex& _v1, Vertex& _v2, Vertex& _v3)
		{
			//Tri sort
			Vertex v1 = _v1;
			Vertex v2 = _v2;
			Vertex v3 = _v3;

			if (TriangleAreaTimesTwo(v1, v3, v2) >= 0)
			{
				//return;
			}

			mat4 screenSpaceTransform;
			InitScreenSpace(screenSpaceTransform, _Context.Width / 2.0f, _Context.Height / 2.0f);

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

			Gradients gradients;
			Edge topToBottom = { &v1, &v3 };
			Edge topToMiddle = { &v1, &v2 };
			Edge middleToBottom = { &v2, &v3 };

			CalculateGradients(gradients, v1, v2, v3);
			CalculateEdge(topToBottom, gradients, 0);
			CalculateEdge(topToMiddle, gradients, 0);
			CalculateEdge(middleToBottom, gradients, 1);

			bool32 handedness = TriangleAreaTimesTwo(v1, v3, v2) >= 0 ? true : false;

			_paScanEdges(gradients, topToBottom, topToMiddle, handedness);
			_paScanEdges(gradients, topToBottom, middleToBottom, handedness);
		}
	}
}