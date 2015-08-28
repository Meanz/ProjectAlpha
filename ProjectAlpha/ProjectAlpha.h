#pragma once

#include "ctdfs.h"
#include "Platform.h"
#include "Memory.h"

using namespace ProjectAlpha;

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

struct PlatformWorkQueue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(PlatformWorkQueue* queue, void* data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(PlatformWorkQueueCallback);

typedef void platform_add_entry(PlatformWorkQueue* queue, PlatformWorkQueueCallback* callback, void* data);
typedef void platform_complete_all_work(PlatformWorkQueue* queue);

struct GameMemory
{
	uint64 permanentMemorySize;
	void* permanentMemory;
	uint64 transientMemorySize;
	void* transientMemory;

	struct PlatformWorkQueue* HighPriorityQueue;
	struct PlatformWorkQueue* LowPriorityQueue;

	platform_add_entry* PlatformAddEntry;
	platform_complete_all_work* PlatformCompleteAllWork;

	Memory::Memory memory;
};


global_variable platform_add_entry *PlatformAddEntry;
global_variable platform_complete_all_work *PlatformCompleteAllWork;

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