### What is ALLOWED:
**All C++98 standard library features:**
- `new` / `delete` operators
- STL containers (vector, map, string, etc.)
- C++ standard headers (`<iostream>`, `<string>`, `<vector>`, `<algorithm>`, etc.)
- `std::memcpy`, `std::strcpy`, etc. from `<cstring>`
- All C++ features and idioms
- All C++98 standard library features are explicitly allowed per the subject

**Only these external/system functions (from subject.md lines 116-124):**
```
execve, pipe, strerror, gai_strerror, errno, dup, dup2, fork, socketpair,
htons, htonl, ntohs, ntohl, select, poll, epoll (epoll_create, epoll_ctl,
epoll_wait), kqueue (kqueue, kevent), socket, accept, listen, send, recv,
chdir, bind, connect, getaddrinfo, freeaddrinfo, setsockopt, getsockname,
getprotobyname, fcntl, close, read, write, waitpid, kill, signal, access,
stat, open, opendir, readdir, closedir
```

### What is FORBIDDEN:
**Any external/system function NOT in the above list**

---

# VIOLATIONS FOUND

