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

		PlatformAddEntry = gameMemory->PlatformAddEntry;
		PlatformCompleteAllWork = gameMemory->PlatformCompleteAllWork;
	}

	__declspec(dllexport) GameUpdateAndRenderDef(_GameUpdateAndRender) {

		if (gameState->pixelBuffer.memory) {
			static real32 val = 0;
			val += 0.001f;
			if (val >= 100) {
				//val = 0.0f;
			}

			//Initialize Matrices

			//PROJECTION_MATRIX
			real32 fovY = ToRadians(70.0f);
			real32 aspect = (real32)gameState->pixelBuffer.width / (real32)gameState->pixelBuffer.height;
			InitPerspective(gameState->Scene.ProjectionMatrix, fovY, aspect, 1.0f, 1000.0f);

			//VIEW_MATRIX
			mat4 _cTrans;
			InitTranslation(_cTrans, vec3(0.0f, 5.0f, -15.0f));

			mat4 _cRot;
			InitRotation(_cRot, vec3(1.0f, 0.0f, 0.0f), (2.0f*Pi32) * 0.2f - 1.25f);

			//gameState->Scene.ViewMatrix = (_cRot * _cTrans);
			InitLookAt(gameState->Scene.ViewMatrix, v3(10.0f, 13.0f, 10.0f), v3(5.0f, 1.0f, 5.0f), v3(0.0f, 1.0f, 0.0f));


			//MODEL_MATRIX

			DemoInit(gameState, gameMemory);
		}
	}
}
