#pragma once

#include <cstddef>
#include <vector>
#include <list>

/**
 * IndexedQueue - O(1) Queue with Key-based Random Access
 * 
 * A queue that supports O(1) operations for:
 * - push(key, data): Add element to back
 * - pop(): Remove element from front
 * - moveToBack(key): Move specific element to back by key
 * - front(): Access front element
 * 
 * Perfect for event loops (epoll/kqueue) where you need to:
 * - Track active connections in order (queue)
 * - Quickly update connection activity by fd (key lookup)
 * - Maintain fairness with round-robin processing
 * 
 * Template Parameters:
 * - K: Key type (e.g., int for file descriptors)
 * - T: Data type (e.g., ClientSocket* for connection objects)
 * 
 * Implementation: std::list + hash table with separate chaining
 * C++98 compatible, uses only standard containers
 */
template <typename K, typename T>
class IndexedQueue
{
private:
	// Queue element holding both data and key
	struct QueueElement
	{
		T data;  // Actual stored data (e.g., ClientSocket*)
		K key;   // Key for lookups (e.g., file descriptor)

		QueueElement(T data, K key)
			: data(data),
			  key(key) {}
	};

	// Hash bucket entry mapping key to list iterator
	// Enables O(1) lookup of queue position by key
	struct HashBucket
	{
		K key;
		typename std::list<QueueElement>::iterator value;

		HashBucket(K k, typename std::list<QueueElement>::iterator v)
			: key(k), value(v) {}
	};

	// Main queue: doubly-linked list for O(1) splice/push/pop
	std::list<QueueElement> queue;
	
	// Hash table: vector of buckets, each bucket is a list (separate chaining)
	// Provides O(1) average lookup by key
	std::vector<std::list<HashBucket> > hashTable;
	size_t tableSize;      // Number of hash buckets
	size_t elementCount;   // Total elements in queue

	// Hash function: converts key to bucket index
	size_t hash(K key) const;
	
	// Find element in hash table, returns queue iterator or queue.end()
	typename std::list<QueueElement>::iterator findInHash(K key) const;
	
	// Insert key->iterator mapping into hash table
	void insertIntoHash(K key, typename std::list<QueueElement>::iterator value);
	
	// Remove key from hash table
	void removeFromHash(K key);
	
	// Rebuild hash table with double size (maintains O(1) average performance)
	void rehash();

public:
	// Constructor: specify initial hash table size (default 16)
	// Use tableSize >= 2 * expected_max_elements for best performance
	IndexedQueue(size_t tableSize = 16);
	
	// Destructor: automatic cleanup (all standard containers)
	~IndexedQueue();

	// Push element to back of queue with associated key
	// If key exists, updates data and moves to back
	// O(1) average
	void push(K key, T data);
	
	// Remove and return front element
	// Throws std::runtime_error if empty
	// O(1) average
	T pop();
	
	// Check if queue is empty
	// O(1)
	bool isEmpty() const;
	
	// Move element with given key to back of queue
	// Used for activity updates in event loops
	// O(1) average
	void moveToBack(K key);
	
	// Access front element without removing
	// Throws std::runtime_error if empty
	// O(1)
	const T front() const;
	
	// Remove element by key from queue
	// Does nothing if key not found
	// O(1) average
	void remove(K key);
};

#include "IndexedQueue.tpp"