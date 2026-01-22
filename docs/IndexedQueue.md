# IndexedQueue: O(1) Queue with Key-Based Access

## Why This Data Structure?

`IndexedQueue` solves a critical problem in event-driven servers using **epoll** (Linux) or **kqueue** (BSD/macOS): efficiently managing active connections with fair scheduling and timeout handling.

## The Problem

In event loops, you need to:
1. **Process connections fairly** - Round-robin/FIFO order
2. **Update activity timestamps** - When data arrives on a socket
3. **Detect timeouts** - Find idle connections quickly
4. **Handle thousands of connections** - Scale efficiently

Standard solutions fail:
- **std::queue**: Can't find/update specific connections → O(n)
- **std::map/set**: No FIFO ordering → unfair scheduling
- **std::vector**: Moving elements is O(n)

## The Solution

`IndexedQueue` combines:
- **std::list** (doubly-linked list) → O(1) push/pop/splice
- **Hash table** (vector of lists) → O(1) key lookup

This gives O(1) for ALL operations:
- Push to back
- Pop from front
- Move specific element to back (by key)

## Event Loop Use Case

### Scenario
You have an HTTP server with epoll handling 1000+ concurrent connections.

### Requirements
1. **Fair Processing**: Handle connections in order they became ready
2. **Activity Tracking**: When socket receives data, mark it as "recently active"
3. **Timeout Detection**: Oldest connection is always at front → check first

### Without IndexedQueue (BAD)
```cpp
std::queue<ClientSocket*> readyQueue;
std::map<int, ClientSocket*> socketMap;  // fd → socket

// When epoll returns ready sockets
for (each ready fd) {
    ClientSocket* client = socketMap[fd];
    
    // Problem: client might already be in queue!
    // Can't efficiently check or move to back
    // Result: duplicate entries or unfair ordering
}

// Processing
ClientSocket* client = readyQueue.front();
readyQueue.pop();
// Problem: Need to search entire queue to update activity
// O(n) search if client becomes ready again!
```

### With IndexedQueue (GOOD)
```cpp
IndexedQueue<int, ClientSocket*> clientQueue(1024);  // fd → ClientSocket*

// When epoll returns ready sockets
for (each ready fd) {
    ClientSocket* client = getClientByFd(fd);
    
    // O(1): If client exists, moves to back; if new, adds to back
    clientQueue.push(client, fd);
}

// Fair round-robin processing
while (!clientQueue.isEmpty()) {
    ClientSocket* client = clientQueue.front();
    clientQueue.pop();  // O(1)
    
    if (client->hasData()) {
        processRequest(client);
        
        // Still active? Move to back for next round
        clientQueue.push(client, client->getFd());  // O(1)
    }
}

// Timeout detection - oldest connection is at front!
if (!clientQueue.isEmpty()) {
    ClientSocket* oldest = clientQueue.front();
    if (oldest->isTimedOut()) {
        clientQueue.pop();  // O(1)
        closeConnection(oldest);
    }
}
```

## Real-World Example: HTTP Server

```cpp
#include "IndexedQueue.hpp"
#include <sys/epoll.h>

class WebServer {
    IndexedQueue<int, ClientSocket*> activeClients;
    int epoll_fd;
    
public:
    WebServer() : activeClients(2048) {  // Expect ~2000 connections
        epoll_fd = epoll_create1(0);
    }
    
    void eventLoop() {
        epoll_event events[128];
        
        while (true) {
            // Wait for events
            int n = epoll_wait(epoll_fd, events, 128, 1000);
            
            // Mark ready connections as active (move to back)
            for (int i = 0; i < n; i++) {
                int fd = events[i].data.fd;
                ClientSocket* client = getClient(fd);
                
                // O(1): Already in queue? Move to back. New? Add to back.
                activeClients.push(client, fd);
            }
            
            // Fair processing: handle oldest active connection first
            if (!activeClients.isEmpty()) {
                ClientSocket* client = activeClients.front();
                activeClients.pop();  // O(1)
                
                // Process one request
                if (handleRequest(client)) {
                    // Keep alive? Re-add to back
                    activeClients.push(client, client->getFd());
                }
            }
            
            // Timeout check: front = oldest connection
            checkTimeouts();
        }
    }
    
    void checkTimeouts() {
        time_t now = time(NULL);
        
        // Check oldest first (front of queue)
        while (!activeClients.isEmpty()) {
            ClientSocket* oldest = activeClients.front();
            
            if (now - oldest->lastActivity < TIMEOUT) {
                break;  // All others are newer
            }
            
            // Timed out - remove and close
            activeClients.pop();
            closeConnection(oldest);
        }
    }
};
```

## Performance Comparison

| Operation | std::queue | std::map | IndexedQueue |
|-----------|------------|----------|--------------|
| Push back | O(1) | O(log n) | **O(1)** |
| Pop front | O(1) | O(log n) | **O(1)** |
| Find by key | O(n) | O(log n) | **O(1)** |
| Move to back | O(n) | O(log n) | **O(1)** |
| Memory | Low | Medium | Medium |

## Why Not std::unordered_map?

`std::unordered_map` (C++11) provides O(1) lookup but:
- Not available in C++98
- Doesn't maintain insertion order
- Can't iterate in FIFO order

`IndexedQueue` gives you both: hash table speed + queue ordering in C++98.

## Key Design Choices

### 1. std::list for Queue
- O(1) push_back, pop_front
- O(1) splice (move element to any position)
- Iterators never invalidate (safe to store in hash table)

### 2. Separate Chaining for Collisions
- `std::vector<std::list<HashBucket>>` instead of linked pointers
- No manual memory management
- C++98 compatible

### 3. Dynamic Rehashing
- Doubles table size when load factor > 0.5
- Keeps chains short (average 0.5 elements)
- Amortized O(1) cost per operation

### 4. Key-Value Design
- Key (int fd): Fast lookup in hash table
- Value (ClientSocket*): Actual data in queue
- Decouples lookup from storage

## When to Use IndexedQueue

✅ **Use when you need:**
- FIFO processing order
- Fast lookup by key (fd, id, etc.)
- Fair scheduling / round-robin
- Timeout detection (oldest = front)
- C++98 compatibility

❌ **Don't use when:**
- You only need FIFO (use std::queue)
- You only need lookup (use std::map)
- Random access by position needed (use std::vector)
- Keys aren't unique

## Summary

`IndexedQueue` is purpose-built for **event loops with epoll/kqueue**:
- Maintains fair FIFO order
- Updates activity in O(1) by file descriptor
- Detects timeouts by checking oldest (front)
- Scales to thousands of connections
- Pure C++98 with standard containers

Perfect for high-performance network servers!
