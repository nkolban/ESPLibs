#ifndef List_h
#define List_h
class List {
public:
	/**
	 * Constructor
	 */
	List();

	/**
	 * Return the number of items in the list.
	 */
	int size();
	/**
	 * Add a new item to the end of the list.
	 */
	void add(void *data);
	/**
	 * Get a specific item in the list.
	 */
	void *get(int index);
	/**
	 * Remove an item from the list by index.
	 */
	void *remove(int index);
private:
	/**
	 * Pointer to the next item.
	 */
	List *m_pNext;
	/**
	 * Pointer to the previous item.
	 */
	List *m_pPrev;
	/**
	 * The data contained in this list item.
	 */
	void *m_pData;
};
#endif
