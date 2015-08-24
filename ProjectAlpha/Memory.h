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

		enum
		{
			Memory_Used = 0x1,
			MemoryReleased = 0x2,
		};

		struct Memory
		{
			MemoryBlock* Sentinel;
		};

		MemoryBlock* InsertBlock(MemoryBlock* prev, uint64 size, void* memory);
		MemoryBlock* FindBlockForSize(Memory memory, uint64 size);
		bool32 MergeIfPossible(Memory memory, MemoryBlock* blockA, MemoryBlock* blockB);
		void MemoryRelease(Memory memory, MemoryBlock* block);
		void MemoryRelease(Memory memory, void* _memory);
		void* MemoryAlloc(Memory memory, uint64 size);
	}
}