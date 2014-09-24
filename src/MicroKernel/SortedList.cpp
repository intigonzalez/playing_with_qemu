#include "SortedList.h"

void CreateSortedList(pSortedList list, void* address, void* end, lessThan* comparator){
	list->data = (void**)address;
	list->comparator = comparator;
	list->length = 0;
	list->maxLength = ((unsigned)end - (unsigned)address)/4;
}

bool AddToList(pSortedList list, void* value){
	if (list->length < list->maxLength)
	{
		unsigned i = 0;
		while (i < list->length && (list->comparator(list->data[i],value)))
			i++;
		for (unsigned j = list->length ; j > i ; j--)
			list->data[j] = list->data[j-1];
		list->data[i] = value;
		list->length++;
		return true;
	}
	else return false;
}

void* ValueAt(pSortedList list, uint index){
	return list->data[index];
}

void RemoveFromListAt(pSortedList list, uint index){
	for (unsigned i = index+1 ; i < list->length ; i++)
		list->data[i-1] = list->data[i];
	list->length--;
}

int IndexOf(pSortedList list, void* value){
	unsigned i = 0;
	while (i < list->length && list->data[i]!=value)
		i++;
		
	return (i == list->length)?-1:i;
}