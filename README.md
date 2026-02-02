# webserv

*This project has been created as part of the 42 curriculum by ataher, mkurkar, yaltayeh.*


## Authors

<div align="center">
    <div style="display: inline-block; text-align: center; margin: 0 20px;">
        <img src="images/ataher.jpg" alt="Ahmed Taher" width="200" height="200" style="object-fit: cover;">
        <p><strong>Ahmed Taher</strong><br>@ataher<br>
        <a href="https://github.com/ahmedhodiani">
            <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" alt="GitHub">
        </a>
        <a href="https://profile.intra.42.fr/users/ataher">
            <img src="https://img.shields.io/badge/42-000000?style=flat&logo=42&logoColor=white" alt="42 Profile">
        </a>
        </p>
    </div>
    <div style="display: inline-block; text-align: center; margin: 0 20px;">
        <img src="images/mkurkar.jpg" alt="Muhammad Kurkar" width="200" height="200" style="object-fit: cover;">
        <p><strong>Muhammad Kurkar</strong><br>@mkurkar<br>
        <a href="https://github.com/mkurkar">
            <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" alt="GitHub">
        </a>
        <a href="https://profile.intra.42.fr/users/mkurkar">
            <img src="https://img.shields.io/badge/42-000000?style=flat&logo=42&logoColor=white" alt="42 Profile">
        </a>
        </p>
    </div>
    <div style="display: inline-block; text-align: center; margin: 0 20px;">
        <img src="images/yaltayeh.jpg" alt="Yacoub Altayeh" width="200" height="200" style="object-fit: cover;">
        <p><strong>Yacoub Altayeh</strong><br>@yaltayeh<br>
        <a href="https://github.com/yaltayeh">
            <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" alt="GitHub">
        </a>
        <a href="https://profile.intra.42.fr/users/yaltayeh">
            <img src="https://img.shields.io/badge/42-000000?style=flat&logo=42&logoColor=white" alt="42 Profile">
        </a>
        </p>
    </div>
</div>





## Description

**webserv** is a fully functional HTTP/1.0 server implementation written in C++98, inspired by nginx. This project demonstrates low-level network programming, event-driven architecture, and HTTP protocol implementation from scratch.

The server features:
- **Event-driven architecture** using epoll (Linux) or kqueue (macOS) for efficient I/O multiplexing
- **Virtual host support** for serving multiple domains from a single server instance
- **CGI/1.0 support** for executing dynamic content (Python, shell scripts, etc.)
- **Nginx-like configuration syntax** with hierarchical directive system
- **Middleware pipeline** inspired by Express.js for request processing
- **Non-blocking I/O** for handling concurrent connections efficiently
- **Comprehensive HTTP features**: GET, POST, DELETE methods, chunked transfer encoding, multipart form data, file uploads, custom error pages, URL redirection, directory listing (autoindex), and more

The project is structured around three core internal libraries:
- **libhttp**: HTTP protocol parsing and response generation
- **libutils**: Logging system and utility functions
- **libdir**: Configuration file parser with directive system inspired by `ffmpeg` filters

### Architecture Highlights

- **Socket Hierarchy**: `ListenSocket` for accepting connections, `ClientSocket` for HTTP handling, `StreamSocket` for CGI communication
- **EventLoop**: Central event dispatcher managing all I/O events
- **VirtualServer**: Routes requests to appropriate server blocks based on Host header
- **Middleware Chain**: Modular request processing (access control → method validation → CGI → static files → autoindex)

## Instructions

### Compilation

```bash
# Clean build from scratch
make fclean && make

# Build with custom buffer size (default: 8192 bytes)
make BUFFER_SIZE=16384

# Build the dependencies (libhttp, libutils, libdir) then does make all
make deps

# Clean build artifacts (keeps binaries)
make clean

# Remove all build artifacts and binaries
make fclean

# Rebuild everything
make re
```

The compiled binary will be located at `build/webserv`.

### Running the Server

```bash
# Start the server with an optional configuration file
./build/webserv [config-file]

# Example: Using the default configuration
./build/webserv webserv.conf

# Example: Using an example configuration
./build/webserv examples/example1/example1.conf
```

See `webserv.conf` and files in `examples/` for configuration examples.

#### Key Directives

- **Global/HTTP context**: `log_level`, `log_colored`, `log_show_location`, `idle_timeout`
- **Server context**: `listen`, `server_name`, `root`, `error_page`, `client_max_body_size`
- **Location context**: `allowed_methods`, `index`, `autoindex`, `return` (redirects), `cgi_pass`, `upload_path`

### Testing

<div align="center">
    <img src="https://c.files.bbci.co.uk/0c25/live/6f1543a0-3719-11f0-96c3-cf669419a2b0.jpg" alt="Testing" width="600">
</div>

The project includes a comprehensive Python test framework (`luka`) with 130+ test suites covering:

- Basic HTTP functionality (GET, POST, DELETE)
- CGI execution and environment variables
- Autoindex and directory listings
- Error page handling
- Method restrictions and access control
- Concurrent connections and large file transfers
- URL encoding and edge cases
- Security (path traversal prevention)
- Configuration validation
- Multi-server virtual hosting

#### Running Tests

```bash
# Run the complete test suite
python3 luka/runner.py

# with valgrind to check for memory leaks
python3 luka/runner.py -v

# compact output
python3 luka/runner.py -c

# extract a test to run individually
python3 luka/extract_test.py <test_name>
```

## Resources

### HTTP Protocol

- [RFC 2616 - HTTP/1.1](https://www.ietf.org/rfc/rfc2616.txt) - The HTTP/1.1 specification
- [RFC 3875 - CGI/1.1](https://www.ietf.org/rfc/rfc3875.txt) - Common Gateway Interface specification
- [MDN HTTP Documentation](https://developer.mozilla.org/en-US/docs/Web/HTTP) - Comprehensive HTTP reference
- [HTTP Made Really Easy](https://www.jmarshall.com/easy/http/) - Beginner-friendly HTTP tutorial

### Network Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Practical guide to socket programming in C
- [The C10K Problem](http://www.kegel.com/c10k.html) - Understanding scalable I/O event handling
- [epoll(7) man page](https://man7.org/linux/man-pages/man7/epoll.7.html) - Linux epoll documentation
- [kqueue(2) man page](https://man.freebsd.org/cgi/man.cgi?query=kqueue&sektion=2) - BSD kqueue documentation

### C++ and Design Patterns

- [C++ 98 Standard](https://www.lirmm.fr/~ducour/Doc-objets/ISO+IEC+14882-1998.pdf) - ISO C++ 1998 specification
- [Middleware Pattern](https://expressjs.com/en/guide/using-middleware.html) - Express.js middleware concept (applied in this project)
- [Factory Pattern](https://refactoring.guru/design-patterns/factory-method) - Used in directive system

### nginx

- [nginx Configuration Documentation](https://nginx.org/en/docs/) - Inspiration for configuration syntax
- [nginx Architecture](https://www.aosabook.org/en/nginx.html) - Understanding nginx's event-driven model

---

### AI Usage

AI tools were used in this project for:
- Research and Learning: Understanding HTTP/1.0 protocol specifications and CGI implementation
- Configuration Guidance: Reviewing nginx configuration syntax and directive patterns
- Documentation: Structuring and organizing project documentation and code comments
- Troubleshooting: Debugging non-blocking I/O issues, memory leaks, and socket management problems

AI was NOT used for:
- Direct code generation without understanding
- Core event loop implementation
- Bypassing the learning process and fundamental architecture decisions

All implementations were reviewed, understood, and customized for this specific project's requirements.

---

## Authors

- **ataher** - Ahmed Taher
- **mkurkar** - Muhammad Kurkar  
- **yaltayeh** - Yacoub Altayeh

## License

This project is part of the 42 Network curriculum and follows the school's academic policies.