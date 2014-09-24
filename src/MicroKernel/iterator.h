#ifndef ITERATOR_H
#define ITERATOR_H

class Iterator {

public:
	virtual bool HasMore() = 0;
	virtual void* Current() = 0;
	virtual void Next() = 0;

};

#endif // ITERATOR_H
