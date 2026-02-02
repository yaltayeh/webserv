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

## Summary
Found **3 violations** across 2 files using forbidden functions not in the allowed list.

**Status:** ✓ `perror()` violations FIXED | `_exit()` violations remain

---

## 1. `perror()` - NOT ALLOWED ✓ FIXED

### File: `src/EventLoop.cpp`
- **Line 340:** ~~`perror("epoll_ctl MOD");`~~ **REMOVED**
- **Line 347:** ~~`perror("epoll_ctl ADD");`~~ **REMOVED**

**Issue:** `perror()` is NOT in the allowed functions list from subject.md.

**Fix Applied:** Removed both `perror()` calls since error messages are already logged using `strerror(errno)` in LOG_ERROR above them.

✓ **FIXED**

---

## 2. `atoi()` - STYLE ISSUE (Not a violation, but inconsistent)

### File: `src/routing/middleware/CGIMiddleware.cpp`
- **Line 131:** `int server_port = server_port_str.empty() ? 8080 : atoi(server_port_str.c_str());`

**Note:** `atoi()` from `<cstdlib>` IS part of C++98 standard library and is ALLOWED. However, it's better to use `std::atoi()` for consistency with C++ style.

**Not a violation - just a style recommendation.**

---

## 3. `std::atoi()` and `std::atoll()` - ✓ ALLOWED

### Files:
- `src/directives/ReturnDirective.cpp` (lines 30, 44)
- `src/directives/ClientMaxBodySizeDirective.cpp` (line 70)
- `src/directives/IdleTimeoutDirective.cpp` (line 61)

**✓ These are ALLOWED** - `std::atoi()` and `std::atoll()` are part of `<cstdlib>` which is the C++98 standard library.

**NOT VIOLATIONS.**

---

## 4. `_exit()` - NOT ALLOWED ⚠️ NOT FIXED YET

### File: `dependencies/libhttp/src/CGI.cpp`
- **Line 502:** `_exit(1);` - after failed dup2() for stdin
- **Line 508:** `_exit(1);` - after failed dup2() for stdout
- **Line 523:** `_exit(1);` - after failed chdir()
- **Line 544:** `_exit(1);` - after failed execve()

**Issue:** `_exit()` is a POSIX system call, NOT part of C++98 standard library, and NOT in the allowed external functions list.

**Context:** These are in the child process after `fork()` in CGI execution.

**Allowed Alternative:**
- **Use `std::exit()` or `exit()` from `<cstdlib>`** - Part of C++98 standard library, explicitly allowed

**Note:** While `_exit()` is traditionally preferred in child processes to avoid flushing stdio buffers, it's a system call not in the allowed list. Must use `std::exit()` instead.

**Recommendation:** Replace all `_exit(1)` with `std::exit(1)`.

---

## Recommended Fixes

### For `perror()` violations: ✓ FIXED
~~Simply remove the `perror()` calls since `strerror(errno)` is already being used in adjacent LOG_ERROR calls.~~

Both `perror()` calls have been removed from `src/EventLoop.cpp`.

### For `_exit()` violations:
Replace with `std::exit()` from `<cstdlib>`:

```cpp
// OLD (NOT ALLOWED):
_exit(1);

// NEW (ALLOWED):
std::exit(1);
```

### Note on atoi/atoll:
**These are ALLOWED** - `std::atoi()`, `std::atoll()` are all part of C++98 standard library (`<cstdlib>`) and are explicitly allowed. No changes needed.

