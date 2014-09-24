#ifndef QUEUE_H
#define QUEUE_H

#include "iterator.h"

class Queue;
typedef Queue* pQueue;

class QueueIterator : public Iterator{
private:
	pQueue q;
	int pos;
public:
	QueueIterator(pQueue queue);
	~QueueIterator();

	bool HasMore();
	void* Current();
	void Next();
};

class Queue {
private:
	int MaxCount;
	int front;
	int end;
	unsigned* data;
public:
	Queue();
	Queue(int maxCount);
	~Queue();
	
	bool Empty() { return front == end; }
	void Enqueue(void* value);
	void* Dequeue();
	void* Front();
	
	Iterator* iterator(); 

	// funciones amigas
	friend bool QueueIterator::HasMore();
	friend void* QueueIterator::Current();
	friend void QueueIterator::Next();
	friend QueueIterator::QueueIterator(pQueue queue);
};



#endif // QUEUE_H
