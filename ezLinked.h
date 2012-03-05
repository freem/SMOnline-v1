#ifndef EZLINKED_H
#define EZLINKED_H

#include "global.h"

class element
{
public:
	element();
	~element();
	element* prev;
	element* next;

	template <class T>
	T data;
}; 

template <class T>
class ezLinked
{
public:
	ezLinked();
	~ezLinked();
	bool PushBack(const T data);
	bool PushFront(const T data);
	T PopBack();
	T PopFront();
private:
	element* m_list;
	element* m_front;
	element* m_end;
};

#endif