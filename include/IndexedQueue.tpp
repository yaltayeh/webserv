#include <IndexedQueue.hpp>
#include <stdexcept>

// Constructor: initialize hash table with given size
template <typename K, typename T>
inline IndexedQueue<K, T>::IndexedQueue(size_t tableSize)
	: tableSize(tableSize),
	  elementCount(0)
{
	hashTable.resize(tableSize);
}

// Destructor: automatic cleanup via standard containers
template <typename K, typename T>
inline IndexedQueue<K, T>::~IndexedQueue()
{
	// std::list and std::vector handle all memory management
}

// Hash function: byte-level hashing with prime multiplier
// Works for any type K (int, pointer, struct, etc.)
template <typename K, typename T>
inline size_t IndexedQueue<K, T>::hash(K key) const
{
	size_t hashValue = 0;
	const char *ptr = reinterpret_cast<const char*>(&key);
	// Iterate over each byte of the key
	for (size_t i = 0; i < sizeof(K); ++i)
	{
		hashValue = hashValue * 31 + ptr[i];  // Prime multiplier for better distribution
	}
	return hashValue % tableSize;  // Map to bucket index
}

// Find element by key in hash table
// Returns iterator to queue element, or queue.end() if not found
// O(1) average - linear search in small bucket list
template <typename K, typename T>
inline typename std::list<typename IndexedQueue<K, T>::QueueElement>::iterator IndexedQueue<K, T>::findInHash(K key) const
{
	size_t index = hash(key);
	const std::list<HashBucket>& bucket = hashTable[index];
	
	// Linear search in bucket (short chain with good load factor)
	for (typename std::list<HashBucket>::const_iterator it = bucket.begin(); it != bucket.end(); ++it)
	{
		if (it->key == key)
			return it->value;  // Return iterator to element in queue
	}
	// Cast queue.end() to non-const iterator (safe since we're not modifying through it)
	return const_cast<std::list<QueueElement>&>(queue).end();  // Not found
}

// Insert key->iterator mapping into hash table
// Rehashes if load factor exceeds 0.5 to maintain O(1) performance
template <typename K, typename T>
inline void IndexedQueue<K, T>::insertIntoHash(K key, typename std::list<QueueElement>::iterator value)
{
	// Rehash when load factor > 0.5 (keeps bucket chains short)
	if (elementCount * 2 > tableSize)
		rehash();
	
	size_t index = hash(key);
	hashTable[index].push_back(HashBucket(key, value));  // Add to bucket chain
	elementCount++;
}

// Remove key from hash table
// O(1) average - linear search in small bucket
template <typename K, typename T>
inline void IndexedQueue<K, T>::removeFromHash(K key)
{
	size_t index = hash(key);
	std::list<HashBucket>& bucket = hashTable[index];
	
	// Find and remove from bucket chain
	for (typename std::list<HashBucket>::iterator it = bucket.begin(); it != bucket.end(); ++it)
	{
		if (it->key == key)
		{
			bucket.erase(it);  // O(1) list erase with iterator
			elementCount--;
			return;
		}
	}
}

// Rebuild hash table with double size
// Called when load factor exceeds 0.5
// O(n) but rare - amortized O(1) cost per insert
template <typename K, typename T>
inline void IndexedQueue<K, T>::rehash()
{
	size_t oldSize = tableSize;
	std::vector<std::list<HashBucket> > oldTable = hashTable;
	
	// Double the table size for better distribution
	tableSize *= 2;
	hashTable.clear();
	hashTable.resize(tableSize);
	elementCount = 0;
	
	// Reinsert all entries with new hash values
	for (size_t i = 0; i < oldSize; ++i)
	{
		for (typename std::list<HashBucket>::iterator it = oldTable[i].begin(); 
		     it != oldTable[i].end(); ++it)
		{
			insertIntoHash(it->key, it->value);  // Recalculates hash
		}
	}
}

// Push element to back of queue
// If key exists: updates data and moves to back
// If new: adds to back and indexes in hash table
// O(1) average
template <typename K, typename T>
inline void IndexedQueue<K, T>::push(K key, T data)
{
	// Check if key already exists
	typename std::list<QueueElement>::iterator existing = findInHash(key);
	if (existing != const_cast<std::list<QueueElement>&>(queue).end())
	{
		existing->data = data;  // Update with new data
		moveToBack(key);        // Move to back (mark as most recent)
		return;
	}

	// Add new element to back of queue
	queue.push_back(QueueElement(data, key));
	typename std::list<QueueElement>::iterator it = queue.end();
	--it;  // Get iterator to newly added element
	
	// Index in hash table for O(1) lookup
	insertIntoHash(key, it);
}

// Remove and return front element
// O(1) average
template <typename K, typename T>
inline T IndexedQueue<K, T>::pop()
{
	if (isEmpty())
		throw std::runtime_error("IndexedQueue is empty");
	
	// Get data and key from front
	T data = queue.front().data;
	K key = queue.front().key;
	
	// Remove from queue
	queue.pop_front();  // O(1) for std::list
	
	// Remove from hash table index
	removeFromHash(key);  // O(1) average
	
	return data;
}

// Check if queue is empty
// O(1)
template <typename K, typename T>
inline bool IndexedQueue<K, T>::isEmpty() const
{
	return queue.empty();
}

// Move element to back of queue by key
// Critical for event loops: marks connection as recently active
// O(1) average
template <typename K, typename T>
inline void IndexedQueue<K, T>::moveToBack(K key)
{
	// Find element in O(1) average using hash table
	typename std::list<QueueElement>::iterator it = findInHash(key);
	if (it == const_cast<std::list<QueueElement>&>(queue).end())
		return;  // Key not found
	
	// Check if already at back (optimization)
	typename std::list<QueueElement>::iterator lastIt = queue.end();
	--lastIt;
	if (it == lastIt)
		return;  // Already at back, nothing to do
	
	// Move to back using splice: O(1) operation in std::list
	// splice(position, source_list, element) moves element to position
	queue.splice(queue.end(), queue, it);
}

// Access front element without removing
// O(1)
template <typename K, typename T>
inline const T IndexedQueue<K, T>::front() const
{
	if (isEmpty())
		throw std::runtime_error("IndexedQueue is empty");
	return queue.front().data;
}

// Remove element by key from queue
// O(1) average - uses hash table lookup + list erase
template <typename K, typename T>
inline int IndexedQueue<K, T>::remove(K key)
{
	// Find element in O(1) average using hash table
	typename std::list<QueueElement>::iterator it = findInHash(key);
	if (it == const_cast<std::list<QueueElement>&>(queue).end())
		return 0;  // Key not found, nothing to do
	
	// Remove from queue (O(1) for std::list with iterator)
	queue.erase(it);
	
	// Remove from hash table index
	removeFromHash(key);
	return 1;
}

template <typename K, typename T>
inline size_t IndexedQueue<K, T>::size() const
{
	return queue.size();
}
