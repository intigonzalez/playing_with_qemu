#include "queue.h"

#include "video.h"

/**
 * \brief Crea una cola con tamaño inicial de 100
 */
Queue::Queue()
{
	MaxCount = 100;
	front = 0;
	end = 0;
	data = new unsigned[MaxCount];
}

Queue::~Queue()
{
	delete[] data;
}

/**
 * \brief Crea una cola con un tamaño inicial prefijado
 * \param maxCount - Tamaño de la cola
 */
Queue::Queue(int maxCount)
{
	MaxCount = maxCount;
	front = 0;
	end = 0;
	data = new unsigned[MaxCount];
}

/**
 * \brief Inserta un elemento en la cola
 * \param value
 */
void Queue::Enqueue(void* value)
{
	// Siempre dejamos un espacio por el medio
	if ((end+1)%MaxCount == front)
	{
		// si no hay espacio reservamos
		unsigned* temp = new unsigned[MaxCount*2];
		int i;
		for (i = 0; front != end ; i++)
		{
			temp[i] = data[front];
			front = (front + 1) % MaxCount;
		}
		front = 0;
		end = i;
		delete [] data;
		data = temp;
		MaxCount *= 2; 
 	}

	data[end] = (unsigned)value;
	end = (end + 1) % MaxCount;
}

/**
 * \brief Saca el primer elemento de la cola
 */
void* Queue::Dequeue()
{
	if (front != end)
	{
		// si hay alguien en la cola
		void * v = (void*)data[front];
		front = (front + 1) % MaxCount;
		return v;
	}
	return 0;
}

/**
 * \brief Toma el primero de la cola pero sin sacarlo
 * \return 
 */
void* Queue::Front()
{
	if (front != end)
	{
		// si hay alguien en la cola
		void * v = (void*)data[front];
		return v;
	}
	return 0;
}

Iterator* Queue::iterator()
{
	return new QueueIterator(this);
}

// La implementacion del iterador

QueueIterator::QueueIterator(pQueue queue)
{
	q = queue;
	pos = queue->front;
}

QueueIterator::~QueueIterator()
{
}

bool QueueIterator::HasMore()
{
	return pos != q->end;
}

void* QueueIterator::Current()
{
	return (void*)q->data[pos];
}

void QueueIterator::Next()
{
	pos = (pos+1) % q->MaxCount;
}
