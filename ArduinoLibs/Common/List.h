#ifndef List_h
#define List_h

#include <Arduino.h>

class List {
public:
	List();
	~List();
	int add(void *data);
	void remove(int i);
	int size();
	void *get(int i);

private:
	int m_size;
	int m_free;
	void **m_array;
};

#endif
