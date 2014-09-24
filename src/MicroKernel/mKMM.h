#ifndef _KERNEL_MEMORY_MANAGER_
#define _KERNEL_MEMORY_MANAGER_

#include "includes/global.h"
#include "kernel.h"

#include "SortedList.h"

typedef struct
{
	void * initAddress;
	void * realInit;
	void * endAddress;
	SortedList list;
} Heap, *pHeap;

typedef struct
{
	void * initAddress;
	void * dataAddress;
	void * endAddress;
	uint pageCount;
	int index;
} PageAllocator, * pPageAllocator;

bool CreateHeap(pHeap heap, void* begin, void* end, uint maxEntryCount);
void* kmalloc(pHeap heap, size_t amount);
bool kfree(pHeap heap, void* address);

bool CreatePageAllocator(pPageAllocator allocator, void* begin, void* end);
void* AllocPage(pPageAllocator allocator);
bool DeallocPage(pPageAllocator allocator, void * page);

#endif
