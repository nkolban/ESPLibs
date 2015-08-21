#include "List.h"
/**
 * Constructor
 */
List::List() {
	m_pPrev = 0;
	m_pNext = 0;
}

/**
 * Return the number of items in the list.
 */
int List::size() {
	if (m_pPrev != 0) {
		return -1;
	}
	int size = 0;
	List *pCurrent = this->m_pNext;
	while (pCurrent != 0) {
		size++;
		pCurrent = pCurrent->m_pNext;
	}
	return size;
} // End of size


/**
 * Add a new item to the end of the list.
 */
void List::add(void *data) {
	List *pCurrent = this;
	while (pCurrent->m_pNext != 0) {
		pCurrent = pCurrent->m_pNext;
	}
	List *pNew = new List();
	pNew->m_pPrev = pCurrent;
	pNew->m_pNext = 0;
	pNew->m_pData = data;
	pCurrent->m_pNext = pNew;
} // End of add


/**
 * Get a specific item in the list.
 */
void *List::get(int index) {
	if (m_pPrev != 0) {
		return 0;
	}
	List *pCurrent = m_pNext;
	while (pCurrent != 0 && index != 0) {
		pCurrent = pCurrent->m_pNext;
		index--;
	}
	if (pCurrent == 0) {
		return 0;
	}
	return pCurrent->m_pData;
} // End of get


/**
 * Remove an item from the list by index.
 */
void *List::remove(int index) {
	if (m_pPrev != 0) {
		return 0;
	}
	List *pCurrent = m_pNext;
	while (pCurrent != 0 && index != 0) {
		pCurrent = pCurrent->m_pNext;
		index--;
	}
	if (pCurrent == 0) {
		return 0;
	}
	List *pPrev = pCurrent->m_pPrev;
	List *pNext = pCurrent->m_pNext;
	pPrev->m_pNext = pNext;
	pNext->m_pPrev = pPrev;
	void *pData = pCurrent->m_pData;
	delete pCurrent;
	return pData;
} //End of remove
// End of file
