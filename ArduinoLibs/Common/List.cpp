#include <List.h>
List::List() {
	m_size = 0;
	m_free = 10;
	m_array = (void **)malloc(sizeof(void *) * m_free);
	if (m_array == NULL) {
		Serial1.println("Error with malloc(1)");
	}
}

List::~List() {
	free(m_array);
}

int List::add(void *data) {
	if (m_free == 0) {
		void *temp = (void *)malloc(sizeof(void *) * (m_size + 10));
		if (temp == NULL) {
			Serial1.println("Error with malloc(2)");
		}
		memcpy(temp, m_array, sizeof(void *) * m_size);
		m_free = 10;
		free(m_array);
		m_array = (void **)temp;
	}
	int i = m_size;
	m_array[i] = data;
	m_size++;
	m_free--;
	return i;
}

void List::remove(int i) {
	if (i>= m_size) {
		return;
	}
	while (i<m_size) {
		m_array[i] = m_array[i+1];
		i++;
	}
	m_size--;
	m_free++;
}

int List::size() {
	return m_size;
}

void *List::get(int i) {
	if (i<=m_size) {
		return m_array[i];
	}
	return NULL;
}
