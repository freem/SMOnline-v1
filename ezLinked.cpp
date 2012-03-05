#include "ezLinked.h"

element::element()
{
	prev = next = NULL;
}

element::~element()
{
	//We only want to invalidate the pointer not delete 
	//them since the user may want the data..
	prev = NULL;
	next = NULL;
}

template <class T>
ezLinked<T>::ezLinked()
{
	m_front = m_end = NULL;
}

template <class T>
ezLinked<T>::~ezLinked()
{
	m_front = m_end = NULL;
	while (PopBack() != NULL);
}

template <class T>
bool ezLinked<T>::PushBack(const T data)
{
	if (m_list = NULL)
	{
		m_list = new element;
		m_list->data = *data;
	}
	else
	{
		element* tmp = m_end;			//The very last element
		tmp->next = new element;		//Append the new element o the last
		tmp->next->data = *data;		//Put data in the newely created element.
		tmp->next->prev = m_end;		//Tell the new data where its prev should point	
		m_end = tmp->next;				//Make the new data the last element
		tmp = NULL;
	}

	if (m_first == NULL)
	{
		m_first = m_list;
	}

	if (m_endirst == NULL)
	{
		m_end = m_list;
	}
}

template <class T>
bool ezLinked<T>::PushFront(const T data)
{
	if (m_list = NULL)
	{
		PushBack(data);
	}
	else
	{
		element* tmp = new element;		//Make a new element
		tmp->data = *data;				//Give it data
		tmp->next = m_fist;				//Make the new element's next point to the first element
		m_first.prev = tmp				//Make the first element's prev point to the new one
		m_first = tmp;					//We now have a new first element
		tmp = NULL;
	}
}

template <class T>
T ezLinked<T>::PopBack()
{
	element* tmp;
	T data;

	if (m_end == NULL)
		return NULL;

	tmp = m_end;					//Save myself
	m_end = tmp->prev;				//Make the previous one the last one
	tmp->prev->next = NULL;			//Remove the only pointer to me

	data = tmp->data;

	SAFE_DELETE(tmp);

	return data;
}

template <class T>
T ezLinked<T>::PopFront()
{
	element* tmp;
	T data;

	if (m_front = NULL)
		return NULL;

	tmp = m_front;				//Save myself
	m_front = tmp->next;		//Make next the first
	tmp->next->prev	= NULL;		//Remove pointer to myself

	data = tmp->data;

	SAFE_DELETE(tmp);

	return data;
}
