#pragma once

#include "ctdfs.h"

namespace ProjectAlpha
{
	namespace Memory
	{
		struct MemoryBlock
		{
			MemoryBlock* Prev;
			MemoryBlock* Next;
			uint64 Flags;
			uint64 Size;
		};

		//Flags for the memory block
		enum
		{
			Memory_Used = 0x1,
		};

		//Memory structure
		struct Memory
		{
			MemoryBlock* Sentinel;
		};

		//
		// Basic General Purpose Allocation
		//
		// Memory is divided into chunks, each chunk has a header (MemoryBlock)
		// Each chunk is a part of a circular linked list
		// Allocation subdivides chunks into ChunkSize-RequestSize with a min threshold of 4k
		// When free is called, a chunk-1 chunk+1 merge is attempted, memory is stored continuously
		MemoryBlock* InsertBlock(MemoryBlock* prev, uint64 size, void* memory);
		MemoryBlock* FindBlockForSize(Memory memory, uint64 size);
		bool32 MergeIfPossible(Memory memory, MemoryBlock* blockA, MemoryBlock* blockB);
		void MemoryRelease(Memory memory, MemoryBlock* block);
		void MemoryRelease(Memory memory, void* _memory);
		void* MemoryAlloc(Memory memory, uint64 size);
	}
}