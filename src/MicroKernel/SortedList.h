#ifndef __SORTED_LIST__
#define __SORTED_LIST__

#include "includes/global.h"

typedef bool *lessThan(void*,void*);

typedef struct
{
	void ** data;
	uint length;
	uint maxLength;
	lessThan* comparator;
} SortedList, *pSortedList;

void CreateSortedList(pSortedList list, void* address, void* end, lessThan* comparator);

bool AddToList(pSortedList list, void* value);

void* ValueAt(pSortedList list, uint index);

void RemoveFromListAt(pSortedList list, uint index);

int IndexOf(pSortedList list, void* value);


#endif