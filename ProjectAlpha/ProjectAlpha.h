#pragma once

#include "ctdfs.h"
#include "Platform.h"
#include "Memory.h"
#include "VecMath.h"

/*

TODO(meanz):

ProjectAlpha
	* Move Platform material over to Platform.h
	* Asset System
	* Bump Pointer Memory Allocator ( Need it for fast alloc/dealloc for our Render Queues, sorting and all other temporary stuff )
		* Things to think about
			- Should we zero memory on alloc?
			- How big should the pool be?
			- Should the pool size be dynamic
			- Should the pool be a subdivision of the current Allocator's arena?

Software Renderer
	* Polygon Clipping
	* Texture Mapping
	* Basic Lighting ( Normals and other attribs )
	* Render Queues
		- Tiled Renderer
			- Tesselation for clipping or pixel by pixel clipping?
		- Vertex Sorting
	* Multi-threaded rendering
		- Align to Cache Lines
		
	* Using _mm thingies, cool intrinsics, listen to Casey Muratori talk about it!

Math
	* SIMD

Terrain ( DEMO )
	* Perlin Noise
	* Proper Normal Computation
	* Tesselation based on view distances ( LOD ) 

*/

using namespace ProjectAlpha;
using namespace ProjectAlpha::Math;

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