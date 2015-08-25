#include "ProjectAlpha.h"

#include <Windows.h>
#include <sstream>
#include <iostream>

#include "SoftwareRenderer.h"
#include "Demo.h"

void* mem_alloc(uint32 size)
{
	void* ptr = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return ptr;
}

void mem_free(uint32 address, uint32 size)
{
	VirtualFree((void*)address, 0, MEM_RELEASE);
}

extern "C"
{
	__declspec(dllexport) GameInitDef(_GameInit) {
		//Setup sentinel
		gameMemory->memory.Sentinel = (Memory::MemoryBlock*)gameMemory->permanentMemory;
		gameMemory->memory.Sentinel->Flags = 0;
		gameMemory->memory.Sentinel->Size = 0;
		gameMemory->memory.Sentinel->Next = gameMemory->memory.Sentinel;
		gameMemory->memory.Sentinel->Prev = gameMemory->memory.Sentinel;

		Memory::InsertBlock(gameMemory->memory.Sentinel,
			gameMemory->permanentMemorySize - sizeof(Memory::MemoryBlock),
			(void*)((uint8*)gameMemory->permanentMemory + sizeof(Memory::MemoryBlock))
			);
	}

	__declspec(dllexport) GameUpdateAndRenderDef(_GameUpdateAndRender) {

		if (gameState->pixelBuffer.memory) {

			//Do bitmap memory modifications
			Renderer::FillRect(gameState->pixelBuffer, 0, 0, gameState->pixelBuffer.width, gameState->pixelBuffer.height, 0x00ffffff);

			

			static real32 val = 0;
			val += 0.01f;
			if (val >= 100) {
				//val = 0.0f;
			}

			//Initialize Matrices

			//PROJECTION_MATRIX
			real32 fovY = glm::radians(70.0f);
			real32 aspect = (real32)gameState->pixelBuffer.width / (real32)gameState->pixelBuffer.height;
			gameState->Scene.ProjectionMatrix = glm::perspective(fovY, aspect, 0.1f, 1000.0f);

			//VIEW_MATRIX
			mat4 _cTrans = glm::translate(mat4(1.0f), vec3(0.0f, -1.0f, -10.0f));
			mat4 _cRot = glm::rotate(mat4(1.0f), (2.0f*Pi32) * 0.2f - val, vec3(1.0f, 0.0f, 0.0f));
			gameState->Scene.ViewMatrix = (_cRot * _cTrans);

			//MODEL_MATRIX


			DemoInit(gameState, gameMemory);

			using namespace Renderer;
			//Do scan line thingy
			Vertex v1 = { { -1, -1, 0, 1 }, {}, {} };
			Vertex v2 = { { 0, 2, 0, 1 }, {}, {} };
			Vertex v3 = { { 1, -1, 0, 1 }, {}, {} };

			mat4 translation = glm::translate(mat4(1.0f), vec3(2.0f, 0.0f, 0.0f));
			mat4 rotation = glm::rotate(mat4(1.0f), val, vec3(0.0f, 1.0f, 0.0f));

			//InitRotation(rotation, { 0.0f, 1.0f, 0.0f }, ToRadians(val));

			//mvp
			//row major
			mat4 model = (translation * rotation);
			mat4 transform = gameState->Scene.ProjectionMatrix * gameState->Scene.ViewMatrix * model;

			v1.Position = transform * v1.Position;
			v2.Position = transform * v2.Position;
			v3.Position = transform * v3.Position;

			Triangle t = { v1, v2, v3 };
			RenderTriangle(gameState->pixelBuffer, gameState->Scene.ProjectionMatrix, mat4(1.0f), t, 0x000000ff);
		}
	}
}
