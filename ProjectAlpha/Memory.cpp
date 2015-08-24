#include "Memory.h"

namespace ProjectAlpha
{
	namespace Memory
	{

		MemoryBlock* InsertBlock(MemoryBlock* prev, uint64 size, void* memory)
		{
			ASSERT((size > sizeof(MemoryBlock)));
			MemoryBlock* block = (MemoryBlock*)memory;
			block->Flags = 0;
			block->Size = size - sizeof(MemoryBlock);
			block->Prev = prev;
			block->Next = prev->Next;
			block->Prev->Next = block;
			block->Next->Prev = block;
			return block;
		}


		//TODO(meanzie): Optimize, uses O(n) now, the more subdivisons you have the longer this function will take
		MemoryBlock* FindBlockForSize(Memory memory, uint64 size)
		{
			MemoryBlock* result = 0;
			// TODO(meanzie): find best match block!
			for (MemoryBlock* block = &memory.Sentinel; block != &memory.Sentinel; block = block->Next)
			{
				if (!(block->Flags & Memory_Used))
				{
					if (block->Size >= size)
					{
						result = block;
						break;
					}
				}
			}
			return(result);
		}

		bool32 MergeIfPossible(Memory memory, MemoryBlock* blockA, MemoryBlock* blockB)
		{
			if (blockA != &memory.Sentinel &&
				blockB != &memory.Sentinel &&
				(blockA->Flags & Memory_Used || blockB->Flags & Memory_Used)) {
				return ((bool32)false);
			}

			uint8* expectedSecond = (uint8*)blockA + sizeof(MemoryBlock) + blockA->Size;
			if ((uint8*)blockB != expectedSecond)
			{
				return((bool32)false);
			}

			//
			blockA->Size += blockB->Size + sizeof(MemoryBlock);
			blockB->Prev->Next = blockB->Next;
			blockB->Next->Prev = blockA;
			blockB->Flags |= MemoryReleased;
			blockB->Next = blockB->Prev = 0; //NULL
			blockB->Size = 0;

			return((bool32)true);
		}

		void MemoryRelease(Memory memory, MemoryBlock* block)
		{
			block->Flags &= ~Memory_Used;

			//Merge
			if (MergeIfPossible(memory, block->Prev, block))
			{
				block = block->Prev;
			}
			MergeIfPossible(memory, block, block->Next);
		}

		void MemoryRelease(Memory memory, void* _memory)
		{
			MemoryBlock* block = (MemoryBlock*)_memory - 1;
			MemoryRelease(memory, block);
		}

		void* MemoryAlloc(Memory memory, uint64 size)
		{
			void* result = 0;
			for (;;) {
				MemoryBlock* block = FindBlockForSize(memory, size);
				if (block && (size <= block->Size))
				{
					block->Flags |= Memory_Used;
					result = (uint8 *)(block + 1);

					uint64 remainingSize = block->Size - size;
					uint64 blockSplitThreshold = 4096; //1 page
					if (remainingSize > blockSplitThreshold)
					{
						block->Size -= remainingSize;
						InsertBlock(block, remainingSize, (uint8*)result + size);
					}
				}
				else
				{
					//Evict assets
					break;
				}
			}
			return result;
		}
	}
}