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

		//std::stringstream str;
		//str << "Number: " << gameState.number;
		//MessageBox(NULL, str.str().c_str(), "The number", MB_OK);

		if (gameState->pixelBuffer.memory) {

			//Do bitmap memory modifications

			//Create a rect in tl
			for (int32 y = 0; y < gameState->pixelBuffer.height; y++) {
				for (int32 x = 0; x < gameState->pixelBuffer.width; x++) {
					((uint32*)gameState->pixelBuffer.memory)[x + (y * gameState->pixelBuffer.width)] = 0x00ffffff;
				}
			}

		}

		static real32 val = 0;

		//Do scan line thingy
		Vertex v1 = { { -1, -1, 0, 1 }, {}, {} };
		Vertex v2 = { { 0, 1, 0, 1 }, {}, {} };
		Vertex v3 = { { 1, -1, 0, 1 }, {}, {} };

		val += 0.1f;
		if (val >= 100) {
			//val = 0.0f;
		}

		real32 fovY = ToRadians(70);
		real32 aspect = (real32)gameState->pixelBuffer.width / (real32)gameState->pixelBuffer.height;

		mat4 projection;
		InitPerspective(projection, fovY, aspect, 0.1f, 1000.0f);

		mat4 translation;
		InitTranslation(translation, vec3{ 0, 0, 3.0f });

		mat4 rotation;
		InitRotation(rotation, { 0.0f, 1.0f, 0.0f }, ToRadians(val));

		mat4 view;
		InitTranslation(view, vec3{ 0.0f, 0.0f, -3.0f });

		//mvp
		//row major
		mat4 transform = rotation * translation * projection;

		v1.Position = transform * v1.Position;
		v2.Position = transform * v2.Position;
		v3.Position = transform * v3.Position;
		
		RenderTriangle(gameState->pixelBuffer, { v1, v2, v3 });

		//mvp
		//row major
		InitTranslation(translation, vec3{ 0, 0, 4.0f });

		v1 = { { -1, -1, 0, 1 }, {}, {} };
		v2 = { { 0, 1, 0, 1 }, {}, {} };
		v3 = { { 1, -1, 0, 1 }, {}, {} };


		transform = rotation * translation * projection;

		v1.Position = transform * v1.Position;
		v2.Position = transform * v2.Position;
		v3.Position = transform * v3.Position;

		RenderTriangle(gameState->pixelBuffer, { v1, v2, v3 });

		
		DemoInit(gameState, gameMemory);
	}
}
