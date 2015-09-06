#include "Demo.h"
#include "SoftwareRenderer.h"

struct DemoData
{
	vec3* vertices;
	vec3* normals;
	vec2* uvs;
	uint16* indices;
	bool32 isInitialized;
};

global_variable DemoData demoData;

inline r32 GetHeight(i32 x, i32 y)
{
	return sin(x + (y / 10.0f)) * cos(y + (x / 11.0f));
}

inline v3 _CalcNormal_GetPoint(i32 x, i32 y)
{
	return v3((r32)x, GetHeight(x, y), (r32)y);
}

inline v3 CalcTriangleNormal(v3 p1, v3 p2, v3 p3)
{
	return Cross((p3 - p1), (p2 - p1));
}

v3 CalcNormal(i32 x, i32 y)
{
	v3 p1 = _CalcNormal_GetPoint(x, y);
	v3 p2 = _CalcNormal_GetPoint(x, y + 1);
	v3 p3 = _CalcNormal_GetPoint(x + 1, y + 1);
	v3 p4 = _CalcNormal_GetPoint(x + 1, y);
	v3 p5 = _CalcNormal_GetPoint(x, y - 1);
	v3 p6 = _CalcNormal_GetPoint(x - 1, y - 1);
	v3 p7 = _CalcNormal_GetPoint(x - 1, y);

	v3 n1 = CalcTriangleNormal(p3, p4, p1);
	v3 n2 = CalcTriangleNormal(p1, p2, p3);
	v3 n3 = CalcTriangleNormal(p1, p7, p2);
	v3 n4 = CalcTriangleNormal(p1, p6, p6);
	v3 n5 = CalcTriangleNormal(p5, p6, p1);
	v3 n6 = CalcTriangleNormal(p5, p1, p4);
	v3 n7 = CalcTriangleNormal(p4, p1, p3);

	r32 endX = n1.x + n2.x + n3.x + n4.x + n5.x + n6.x + n7.x;
	r32 endY = n1.y + n2.y + n3.y + n4.y + n5.y + n6.y + n7.y;
	r32 endZ = n1.z + n2.z + n3.z + n4.z + n5.z + n6.z + n7.z;

	return v3(endX / 7.0f, endY / 7.0f, endZ / 7.0f);
}

void DemoInit(GameState* state, GameMemory* memory)
{

	//

	int32 width = 12;
	int32 height = 11;
	int32 tileSize = 1;

	int32 numTiles = width * height;
	int32 numVerts = numTiles * 4;
	int32 numIndices = numTiles * 6;

	if (!demoData.isInitialized)
	{
		demoData.vertices = (vec3*)Memory::MemoryAlloc(memory->memory, sizeof(vec3) * numVerts);
		demoData.normals = (vec3*)Memory::MemoryAlloc(memory->memory, sizeof(vec3) * numVerts);
		demoData.uvs = (vec2*)Memory::MemoryAlloc(memory->memory, sizeof(vec2) * numVerts);
		demoData.indices = (uint16*)Memory::MemoryAlloc(memory->memory, sizeof(uint16) * numIndices);

		if (!demoData.vertices || !demoData.indices)
		{
			ASSERT(false);
		}

		vec3* vert = demoData.vertices;
		vec3* norm = demoData.normals;
		vec2* uvs = demoData.uvs;
		uint16* ind = demoData.indices;
		for (int32 x = 0; x < width; x++)
		{
			for (int32 y = 0; y < height; y++)
			{
				//3--2
				//|  |
				//0--1

				//Alloc on stack
				vec3 v1 = { (real32)x + 1, GetHeight(x + 1, y), (real32)y };
				vec3 v2 = { (real32)x, GetHeight(x, y), (real32)y };
				vec3 v3 = { (real32)x, GetHeight(x, y + 1), (real32)y + 1 };
				vec3 v4 = { (real32)x + 1, GetHeight(x + 1, y + 1), (real32)y + 1 };

				//Push to memory
				uint32 v0 = vert - demoData.vertices; //0
				*(vert++) = v1;
				*(vert++) = v2;
				*(vert++) = v3;
				*(vert++) = v4;

				//Normals
				vec3 n1 = CalcNormal(x + 1, y);
				vec3 n2 = CalcNormal(x, y);
				vec3 n3 = CalcNormal(x, y + 1);
				vec3 n4 = CalcNormal(x + 1, y + 1);

				//Push to memory
				*(norm++) = n1;
				*(norm++) = n2;
				*(norm++) = n3;
				*(norm++) = n4;

				//UVs
				vec2 u1 = vec2(0.0f, 1.0f);
				vec2 u2 = vec2(1.0f, 1.0f);
				vec2 u3 = vec2(1.0f, 0.0f);
				vec2 u4 = vec2(0.0f, 0.0f);

				//Push to memory
				*(uvs++) = u1;
				*(uvs++) = u2;
				*(uvs++) = u3;
				*(uvs++) = u4;

				//Indices
				*(ind++) = (uint16)(v0 + 1);
				*(ind++) = (uint16)(v0 + 2);
				*(ind++) = (uint16)(v0 + 0);

				*(ind++) = (uint16)(v0 + 2);
				*(ind++) = (uint16)(v0 + 3);
				*(ind++) = (uint16)(v0 + 0);

			}
		}
		demoData.isInitialized = true;

		//Create context
		Renderer::paCreateContext(memory, state->pixelBuffer.width, state->pixelBuffer.height, 0, &state->pixelBuffer);
	}
	
	Renderer::paSetClearColor(0x00ffffff);
	Renderer::paClear(Renderer::COLOR_BIT | Renderer::DEPTH_BIT);
	Renderer::paSetFillMode(Renderer::PA_FILL);
	Renderer::paEnable(Renderer::PA_DEPTH_TEST);

	static real32 val = 0;
	//val += 0.5f;
	if (val >= 100) {
		//val = 0.0f;
	}

	mat4 translation;
	InitTranslation(translation, vec3(0.0f, 0.0f, 0.0f));
	mat4 rotation;
	InitRotation(rotation, vec3(1.0f, 0.0f, 0.0f), ToRadians(val));

	using namespace Renderer;
	paSetProjectionMatrix(state->Scene.ProjectionMatrix);
	paSetViewMatrix(state->Scene.ViewMatrix);
	mat4 id;
	InitIdentity(id);
	paSetModelMatrix(id);

	for (uint32 i = 0; i < (uint32)numIndices; i += 3)
	{
		uint16 idx = demoData.indices[i];
		uint16 idx1 = demoData.indices[i + 1];
		uint16 idx2 = demoData.indices[i + 2];

		Vertex v1 = { 
			{ demoData.vertices[idx].x, demoData.vertices[idx].y, demoData.vertices[idx].z, 1.0 }, 
			{ demoData.normals[idx].x, demoData.normals[idx].y, demoData.normals[idx].z }, 
			{ demoData.uvs[idx].x, demoData.uvs[idx].y } 
		};
		Vertex v2 = { 
			{ demoData.vertices[idx1].x, demoData.vertices[idx1].y, demoData.vertices[idx1].z, 1.0 }, 
			{ demoData.normals[idx1].x, demoData.normals[idx1].y, demoData.normals[idx1].z },
			{ demoData.uvs[idx1].x, demoData.uvs[idx1].y }
		};
		Vertex v3 = { 
			{ demoData.vertices[idx2].x, demoData.vertices[idx2].y, demoData.vertices[idx2].z, 1.0 }, 
			{ demoData.normals[idx2].x, demoData.normals[idx2].y, demoData.normals[idx2].z },
			{ demoData.uvs[idx2].x, demoData.uvs[idx2].y }
		};

		paSetDrawColor(v4(1.0f, 0.0f, 0.0f, 1.0f));

		Triangle t = { v1, v2, v3 };
		paTriangle(t);
	}

	memory->_cycles = paGetCycles() / paGetCyclesCount();
	paResetCycles();

	//Free the memory
	//Memory::MemoryRelease(memory->memory, vertices);
	//Memory::MemoryRelease(memory->memory, indices);
}