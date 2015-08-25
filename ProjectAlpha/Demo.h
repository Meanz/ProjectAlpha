#pragma once

#include "ProjectAlpha.h"

inline real32 GetHeight(int32 x, int32 y)
{
	return 0.0f;
}

void DemoInit(GameState* gameState, GameMemory* memory)
{

	//

	int32 width = 10;
	int32 height = 10;
	int32 tileSize = 1;

	int32 numTiles = width * height;
	int32 numVerts = numTiles * 4;
	int32 numIndices = numTiles * 6;

	vec3* vertices = (vec3*)Memory::MemoryAlloc(memory->memory, sizeof(vec3) * numVerts);
	uint16* indices = (uint16*)Memory::MemoryAlloc(memory->memory, sizeof(uint16) * numIndices);

	if (!vertices || !indices)
	{
		ASSERT(false);
	}

	vec3* vert = vertices;
	uint16* ind = indices;
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
			uint32 v0 = vert - vertices; //0
			*(vert++) = v1;
			*(vert++) = v2;
			*(vert++) = v3;
			*(vert++) = v4;

			//Indices
			*(ind++) = (uint16)(v0 + 1);
			*(ind++) = (uint16)(v0 + 2);
			*(ind++) = (uint16)(v0 + 0);

			*(ind++) = (uint16)(v0 + 2);
			*(ind++) = (uint16)(v0 + 3);
			*(ind++) = (uint16)(v0 + 0);

		}
	}

	static real32 val = 0;
	val += 0.5f;
	if (val >= 100) {
		//val = 0.0f;
	}

	mat4 translation = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
	mat4 rotation = glm::rotate(mat4(1.0f), glm::radians(val), vec3(1.0f, 0.0f, 0.0f));

	for (uint32 i = 0; i < (uint32)numIndices; i += 3)
	{
		uint16 idx = indices[i];
		uint16 idx1 = indices[i + 1];
		uint16 idx2 = indices[i + 2];

		using namespace Renderer;

		Vertex v1 = { { vertices[idx].x, vertices[idx].y, vertices[idx].z, 1.0 }, {}, {} };
		Vertex v2 = { { vertices[idx1].x, vertices[idx1].y, vertices[idx1].z, 1.0 }, {}, {} };
		Vertex v3 = { { vertices[idx2].x, vertices[idx2].y, vertices[idx2].z, 1.0 }, {}, {} };

		//Create a transform matrix

		//It should be Model * View * Projection
		//Let's see if that actually works :D
		mat4 transform = gameState->Scene.ProjectionMatrix * gameState->Scene.ViewMatrix * (translation);

		v1.Position = transform * v1.Position;
		v2.Position = transform * v2.Position;
		v3.Position = transform * v3.Position;

		Triangle t = { v1, v2, v3 };

		RenderTriangle(gameState->pixelBuffer, gameState->Scene.ProjectionMatrix, mat4(1.0), t, 0x0000ff00);
	}


	//Free the memory
	Memory::MemoryRelease(memory->memory, vertices);
	Memory::MemoryRelease(memory->memory, indices);
}