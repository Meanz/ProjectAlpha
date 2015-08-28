#include "Demo.h"
#include "SoftwareRenderer.h"

struct DemoData
{
	vec3* vertices;
	uint16* indices;
	bool32 isInitialized;
};

global_variable DemoData demoData;

void DemoInit(GameState* state, GameMemory* memory)
{

	//

	int32 width = 10;
	int32 height = 10;
	int32 tileSize = 1;

	int32 numTiles = width * height;
	int32 numVerts = numTiles * 4;
	int32 numIndices = numTiles * 6;

	if (!demoData.isInitialized)
	{
		demoData.vertices = (vec3*)Memory::MemoryAlloc(memory->memory, sizeof(vec3) * numVerts);
		demoData.indices = (uint16*)Memory::MemoryAlloc(memory->memory, sizeof(uint16) * numIndices);

		if (!demoData.vertices || !demoData.indices)
		{
			ASSERT(false);
		}

		vec3* vert = demoData.vertices;
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
	Renderer::_paFillRect(0, 0, state->pixelBuffer.width, state->pixelBuffer.height, 0x00ffffff);

	static real32 val = 0;
	val += 0.5f;
	if (val >= 100) {
		//val = 0.0f;
	}

	mat4 translation = glm::translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
	mat4 rotation = glm::rotate(mat4(1.0f), glm::radians(val), vec3(1.0f, 0.0f, 0.0f));

	for (uint32 i = 0; i < (uint32)numIndices; i += 3)
	{
		uint16 idx = demoData.indices[i];
		uint16 idx1 = demoData.indices[i + 1];

		uint16 idx2 = demoData.indices[i + 2];

		using namespace Renderer;

		Vertex v1 = { { demoData.vertices[idx].x, demoData.vertices[idx].y, demoData.vertices[idx].z, 1.0 }, {}, {} };
		Vertex v2 = { { demoData.vertices[idx1].x, demoData.vertices[idx1].y, demoData.vertices[idx1].z, 1.0 }, {}, {} };
		Vertex v3 = { { demoData.vertices[idx2].x, demoData.vertices[idx2].y, demoData.vertices[idx2].z, 1.0 }, {}, {} };

		//Create a transform matrix

		//It should be Model * View * Projection
		//Let's see if that actually works :D
		mat4 transform = state->Scene.ProjectionMatrix * state->Scene.ViewMatrix * (translation);

		v1.Position = transform * v1.Position;
		v2.Position = transform * v2.Position;
		v3.Position = transform * v3.Position;

		Triangle t = { v1, v2, v3 };


		paSetProjectionMatrix(state->Scene.ProjectionMatrix);
		paSetModelMatrix(mat4(1.0f));
		paTriangle(t, 0x0000ff00);
	}


	//Free the memory
	//Memory::MemoryRelease(memory->memory, vertices);
	//Memory::MemoryRelease(memory->memory, indices);
}