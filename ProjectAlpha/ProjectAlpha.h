#pragma once

#include "ctdfs.h"
#include "Platform.h"
#include "Memory.h"

using namespace ProjectAlpha;

struct GameMemory
{
	uint64 permanentMemorySize;
	void* permanentMemory;
	uint64 transientMemorySize;
	void* transientMemory;

	Memory::Memory memory;
};

struct PixelBuffer
{
	int32 width;
	int32 height;
	uint32 size;
	void* memory;
};

struct GameScene
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

struct GameState
{
	Platform platform;
	PixelBuffer pixelBuffer;
	uint64 number;
	GameScene Scene;
};

void* mem_alloc(uint32 size);
void mem_free(uint32 address, uint32 size);

extern "C"
{
	void GameGetSoundSamples();

#define GameInitDef(name) void name(GameMemory* gameMemory, GameState* gameState)
	typedef GameInitDef(GameInit);
	__declspec(dllexport) GameInitDef(_GameInit);

	#define GameUpdateAndRenderDef(name) void name(GameMemory* gameMemory, GameState* gameState)
	typedef GameUpdateAndRenderDef(GameUpdateAndRender);
	__declspec(dllexport) GameUpdateAndRenderDef(_GameUpdateAndRender);
}

//### TIP OF THE DAY ###
//Typically one of the things you want to try and avoid is passing pointers to things that are on the stack
//Eg you have a struct on the stack and then you pass a pointer of it to another function
//The exception here is when you start getting large structures
//Because the compiler no longer really know what is going on here
//See aliasing
//int x = *B;
//*A = 5;
//int y = *B;
//https://www.youtube.com/watch?v=w7ay7QXmo_o see 5:25 +